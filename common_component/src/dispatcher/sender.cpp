/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include <mooon/utils/string_utils.h>
#include "sender.h"
#include "send_thread.h"
#include "sender_table.h"
#include "default_reply_handler.h"
DISPATCHER_NAMESPACE_BEGIN
    
CSender::CSender()
    :_send_queue(0, this)
    ,_send_thread(NULL)
    ,_sender_table(NULL)
    ,_in_table(false)
    ,_to_shutdown(false)
    ,_cur_resend_times(0)
    ,_current_offset(0)
    ,_current_message(NULL)
{
    /***
      * 默认构造函数，不做实际用，仅为满足CListQueue的空闲头结点需求
      */    
    _sender_info.reply_handler = NULL;
}

CSender::~CSender()
{    
    clear_message();    
    delete _sender_info.reply_handler; // 注意此处的性能
}

CSender::CSender(const SenderInfo& sender_info)
    :_send_queue(sender_info.queue_size, this)
    ,_send_thread(NULL)
    ,_sender_table(NULL)
    ,_to_shutdown(false)
    ,_cur_resend_times(0)
    ,_current_offset(0)
    ,_current_message(NULL)
{   
    set_peer(sender_info.ip_node);
    memcpy(&_sender_info, &sender_info, sizeof(SenderInfo));    

    if (NULL == _sender_info.reply_handler)
        _sender_info.reply_handler = new CDefaultReplyHandler;
}

bool CSender::on_timeout()
{
    return _sender_info.reply_handler->sender_timeout();
}

std::string CSender::to_string() const
{
    return std::string("sender://")
        + util::CStringUtil::int_tostring(_sender_info.key)
        + std::string("-")
        + net::CTcpClient::do_to_string();
}

void CSender::set_reconnect_times(int32_t reconnect_times)
{
	_sender_info.reconnect_times = reconnect_times;
}

void CSender::shutdown()
{
    _to_shutdown = true;
    _sender_info.reconnect_times = 0;
    close_write();
}

void CSender::attach_thread(CSendThread* send_thread)
{ 
    _send_thread = send_thread; 
}

void CSender::attach_sender_table(CSenderTable* sender_table)
{
    _sender_table = sender_table;
}

void CSender::before_close()
{    
    _sender_info.reply_handler->sender_closed();
}

void CSender::after_connect()
{
    _sender_info.reply_handler->sender_connected();
}

void CSender::on_connect_failure()
{
    _sender_info.reply_handler->sender_connect_failure();
}

void CSender::clear_message()
{
    // 删除列队中的所有消息
    message_t* message;
    while (_send_queue.pop_front(message))
    {              
        destroy_message(message);
    }
}

void CSender::inc_resend_times()
{
    ++_cur_resend_times;
}

bool CSender::need_resend() const
{
    return (_sender_info.resend_times < 0) || (_cur_resend_times < _sender_info.resend_times);
}

void CSender::reset_resend_times()
{
    _cur_resend_times = 0;
}

utils::handle_result_t CSender::do_handle_reply()
{    
    size_t buffer_length = _sender_info.reply_handler->get_buffer_length();
    size_t buffer_offset = _sender_info.reply_handler->get_buffer_offset();
    char* buffer = _sender_info.reply_handler->get_buffer();

    // 关闭连接
    if ((buffer_offset >= buffer_length) || (NULL == buffer)) 
    {
        DISPATCHER_LOG_ERROR("%s encountered invalid buffer %"PRIu64":%"PRIu64":%p.\n"
            , to_string().c_str()
            , buffer_length, buffer_offset, buffer);
        return util::handle_error;
    }
    
    ssize_t data_size = this->receive(buffer+buffer_offset, buffer_length-buffer_offset);
    if (0 == data_size) 
    {
        DISPATCHER_LOG_WARN("%s closed by peer.\n", to_string().c_str());
        return util::handle_error; // 连接被关闭
    }

    // 处理应答，如果处理失败则关闭连接
    utils::handle_result_t retval = _sender_info.reply_handler->handle_reply((size_t)data_size);
    if (utils::handle_finish == retval)
    {
        DISPATCHER_LOG_DEBUG("%s reply finished.\n", to_string().c_str());
    }
    else if (utils::handle_error == retval)
    {
        DISPATCHER_LOG_ERROR("%s reply error.\n", to_string().c_str());
    }
    else if (utils::handle_close == retval)
    {
        DISPATCHER_LOG_ERROR("%s reply close.\n", to_string().c_str());
    }

    return retval;
}

bool CSender::get_current_message()
{
    if (_current_message != NULL) return _current_message;
    bool retval = _send_queue.pop_front(_current_message);
    if (retval)
        _sender_info.reply_handler->before_send();

    return retval;
}

void CSender::free_current_message()
{
    reset_resend_times();
    destroy_message(_current_message);
    
    _current_message = NULL;            
    _current_offset = 0;
}

void CSender::reset_current_message(bool finish)
{
    if (_current_message != NULL)
    {    
        if (finish)
        {    
            free_current_message();
        }
        else
        {
            if (need_resend())
            {
                inc_resend_times();
                if (DISPATCH_BUFFER == _current_message->type)
                {         
                    // 如果是dispatch_file不重头发，总是从断点处开始重发
                    _current_offset = 0; // 重头发送
                }
            }
            else
            {
                free_current_message();
            }
        }
    }
}

net::epoll_event_t CSender::do_send_message(void* input_ptr, uint32_t events, void* output_ptr)
{    
    net::CEpoller& epoller = _send_thread->get_epoller();
    
    // 优先处理完本队列中的所有消息
    for (;;)
    {
        if (!get_current_message())
        {
            // 队列里没有了
            epoller.set_events(&_send_queue, EPOLLIN);
            return net::epoll_read;
        }
        
        ssize_t retval;
        if (DISPATCH_FILE == _current_message->type)
        {
            // 发送文件
            file_message_t* file_message = (file_message_t*)(_current_message->data);
            off_t offset = file_message->offset + (off_t)_current_offset; // 从哪里开始发送
            size_t size = _current_message->length - (size_t)offset; // 剩余的大小
            
            net::set_tcp_option(get_fd(), true, TCP_CORK);
            retval = send_file(file_message->fd, &offset, size);
            net::set_tcp_option(get_fd(), false, TCP_CORK);
        }
        else if (DISPATCH_BUFFER == _current_message->type)
        {
            // 发送Buffer
            buffer_message_t* buffer_message = (buffer_message_t*)(_current_message->data);
            retval = send(buffer_message->data+_current_offset, _current_message->length-_current_offset);
        }   
        else
        {
            MYLOG_DEBUG("%s received message %d.\n", to_string().c_str(), _current_message->type);
            free_current_message();
            return net::epoll_close;
        }
        
        if (-1 == retval)
        {
            return net::epoll_read_write; // wouldblock                    
        }

        _current_offset += (size_t)retval;
        _sender_info.reply_handler->send_progress(_current_message->length, _current_offset, (size_t)retval);

        // 未全部发送，需要等待下一轮回
        if (_current_offset < _current_message->length) 
        {
                return net::epoll_read_write;
        }
        
        // 发送完毕，继续下一个消息
        _sender_info.reply_handler->send_completed();
        reset_current_message(true);            
    }  
    
    return net::epoll_close;
}

net::epoll_event_t CSender::handle_epoll_event(void* input_ptr, uint32_t events, void* output_ptr)
{    
    util::CTimeoutManager<CSender>* timeout_manager;
    timeout_manager = get_send_thread()->get_timeout_manager();
    timeout_manager->remove(this);
    
    try
    {
        do
        {           
            if (EPOLLHUP & events)
            {
                if (_to_shutdown)
                {
                    DISPATCHER_LOG_INFO("%s is active closed.\n", to_string().c_str());
                }
                else if (is_connect_establishing())
                {
                    DISPATCHER_LOG_ERROR("%s Connection refused.\n", to_string().c_str());
                }
                else
                {
                    DISPATCHER_LOG_ERROR("%s happen HUP event.\n", to_string().c_str());
                }                
                
                break;
            }             
            else if (EPOLLERR & events)
            {
                DISPATCHER_LOG_ERROR("%s happen ERROR event.\n", to_string().c_str());
                break;
            } 
            else if (EPOLLIN & events)
            {                      
                utils::handle_result_t reply_retval = do_handle_reply();
                if (utils::handle_error == reply_retval)
                {
                    break;
                }                
                if (utils::handle_close == reply_retval)
                {
                    break;
                } 
                if (utils::handle_release == reply_retval)
                {
                    // 通知线程：释放，销毁Sender，不能再使用
                    return net::epoll_destroy;
                }
                
                timeout_manager->push(this, get_send_thread()->get_current_time());
                return net::epoll_none;                
            }
            else if (EPOLLOUT & events)
            {
                // 如果是正在连接，则切换状态
                if (is_connect_establishing())
                {
                	set_connected_state();
                	DISPATCHER_LOG_DEBUG("%s to asynchronously connect sucessfully.\n", to_string().c_str());
                }

                net::epoll_event_t send_retval = do_send_message(input_ptr, events, output_ptr);
                if (net::epoll_close == send_retval) 
                {
                    break;
                }
                
                timeout_manager->push(this, get_send_thread()->get_current_time());
                return send_retval;
            }    
            else // Unknown events
            {
                DISPATCHER_LOG_ERROR("%s got unknown events %d.\n", to_string().c_str(), events);
                break;
            }
        } while (false);
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接异常
        DISPATCHER_LOG_ERROR("%s error for %s.\n", to_string().c_str(), ex.to_string().c_str());
    }

    reset_current_message(false);    
    return net::epoll_close;
}

template <typename ConcreteMessage>
bool CSender::do_push_message(ConcreteMessage* concrete_message, uint32_t milliseconds)
{
    char* message_buffer = reinterpret_cast<char*>(concrete_message) - sizeof(message_t);
    message_t* message = reinterpret_cast<message_t*>(message_buffer);

    return _send_queue.push_back(message, milliseconds);
}

bool CSender::push_message(file_message_t* message, uint32_t milliseconds)
{
    return do_push_message(message, milliseconds);
}

bool CSender::push_message(buffer_message_t* message, uint32_t milliseconds)
{
    return do_push_message(message, milliseconds);
}

DISPATCHER_NAMESPACE_END
