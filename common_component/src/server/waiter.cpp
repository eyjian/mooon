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
 * Author: JianYi, eyjian@qq.com
 */
#include <sstream>
#include <mooon/net/utils.h>
#include <mooon/sys/thread.h>
#include <mooon/utils/string_utils.h>
#include "waiter.h"
#include "work_thread.h"
SERVER_NAMESPACE_BEGIN

CWaiter::CWaiter()
    :_is_sending(false)
    ,_is_in_pool(false) // 只能初始化为false
    ,_thread_index(0)
    ,_packet_handler(NULL)
{
}

CWaiter::~CWaiter()
{
    delete _packet_handler;
    _packet_handler = NULL;
}

void CWaiter::reset()
{
    _is_sending = false;
    _packet_handler->reset();
}

bool CWaiter::on_timeout()
{
    return _packet_handler->on_connection_timeout();
}

void CWaiter::on_switch_failure(bool overflow)
{
    _packet_handler->on_switch_failure(overflow);
}

void CWaiter::before_close()
{
    _packet_handler->on_connection_closed();
}

net::epoll_event_t CWaiter::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{
    net::epoll_event_t retval = net::epoll_close;
    CWorkThread* thread = static_cast<CWorkThread *>(input_ptr);
    thread->update_waiter(this); // 更新时间戳，防止超时
    
    try
    {   
        if (EPOLLHUP & events)
        {
            retval = do_handle_epoll_error((void*)"hang up", ouput_ptr);
        }
        else if (EPOLLERR & events)
        {
            retval = do_handle_epoll_error((void*)"error", ouput_ptr);
        }
        else if (EPOLLIN & events)
        {
            retval = do_handle_epoll_read(input_ptr, ouput_ptr);
        }
        else if (EPOLLOUT & events)
        {
            retval = do_handle_epoll_send(input_ptr, ouput_ptr);
        }
        else
        {
            std::string info = std::string("events - ") + utils::CStringUtils::int_tostring(events);
            retval = do_handle_epoll_error((void*)info.c_str(), ouput_ptr);
        }
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_ERROR("Waiter %s error: %s.\n", to_string().c_str(), ex.to_string().c_str());	

        // 如果是IO错误，则进行回调
        if (EIO == ex.get_errcode())
        {
            _packet_handler->on_io_error();
        }

        return net::epoll_close;
    }

    return retval;
}

net::epoll_event_t CWaiter::do_handle_epoll_send(void* input_ptr, void* ouput_ptr)
{
    size_t size;
    size_t offset;
    ssize_t retval;

    if (!_is_sending)
    {
        _is_sending = true;
        _packet_handler->before_response();
    }

    const ResponseContext* response_context = _packet_handler->get_response_context();
    size = response_context->response_size;
    offset = response_context->response_offset;

    // 无响应数据需要发送
    if (size < offset)
    {
        MYLOG_WARN("Response size %"PRIu64" less than offset %"PRIu64".\n", size, offset);
    }    
    else if (size == offset)
    {
        MYLOG_DEBUG("Response size %"PRIu64" equal to offset %"PRIu64".\n", size, offset);
    }
    else if (size > offset)
    {
        try
        {
            // 发送文件或数据
            if (response_context->is_response_fd)
            {
                // 发送文件
                off_t file_offset = (off_t)offset;
                int file_fd = response_context->response_fd;

                net::set_tcp_option(get_fd(), true, TCP_CORK);
                retval = CTcpWaiter::send_file(file_fd, &file_offset, size-offset);
                net::set_tcp_option(get_fd(), false, TCP_CORK);
            }
            else
            {
                // 发送Buffer
                const char* buffer = response_context->response_buffer;
                if (buffer != NULL)
                {
                    retval = CTcpWaiter::send(buffer+offset, size-offset);
                }
                else
                {
                    SERVER_LOG_ERROR("Response[%zu:%zu] buffer is NULL.\n", size, offset);
                }
            }
        }
        catch (sys::CSyscallException& ex)
        {
            // EINVAL比较容易犯，故特别处理一下，以方便定位问题
            if (EINVAL == ex.get_errcode())
            {
                SERVER_LOG_ERROR("Invalid argument: %s.\n", response_context->to_string().c_str());
            }
            else
            {
                SERVER_LOG_DEBUG("Send argument: %s.\n", response_context->to_string().c_str());
            }

            throw;
        }

        if (-1 == retval)
        {
            // Would block
            //SERVER_LOG_DEBUG("%s send block.\n", to_string().c_str());
            return net::epoll_write;
        }

        // 更新已经发送的大小值
        _packet_handler->move_response_offset((size_t)retval);	
        if (response_context->response_size > response_context->response_offset)
        {
            // 没有发完，需要继续发
            return net::epoll_write;        
        }
    }              

    Indicator indicator;
    indicator.reset = true;
    indicator.thread_index = get_thread_index();
    indicator.epoll_events = EPOLLIN;

    _is_sending = false; // 结束发送状态，再次进入接收状态
    utils::handle_result_t handle_result;

    handle_result = _packet_handler->on_response_completed(indicator);
    if (indicator.reset)
    {
        reset(); // 复位状态，为下一个消息准备
        SERVER_LOG_DETAIL("%s was reset.\n",  str().c_str());
    }

    HandOverParam* handover_param = static_cast<HandOverParam*>(ouput_ptr);
    if (utils::handle_release == handle_result)
    {
        SERVER_LOG_DEBUG("%s will be released.\n", str().c_str());
        handover_param->thread_index = indicator.thread_index;
        handover_param->epoll_events = indicator.epoll_events;
        return net::epoll_release;
    }
    if (utils::handle_continue == handle_result)
    {
        // 这里，忽略handover_param->thread_index
        handover_param->epoll_events = indicator.epoll_events;
        return net::epoll_none;
    }
    else
    {
        SERVER_LOG_DEBUG("Return %d, %s will be closed.\n", handle_result, str().c_str());
        return net::epoll_close;
    }
}

net::epoll_event_t CWaiter::do_handle_epoll_read(void* input_ptr, void* ouput_ptr)
{
    ssize_t retval;
    RequestContext* request_context = _packet_handler->get_request_context();
    size_t buffer_offset = request_context->request_offset;
    size_t buffer_size = request_context->request_size;
    char* buffer = request_context->request_buffer;

#if 0 // 条件成立时，也可能是因为对端关闭了连接
    // 检查参数
    if ((buffer_size == buffer_offset) || (NULL == buffer))
    {
        SERVER_LOG_ERROR("Waiter %s encountered invalid buffer %"PRIu64":%p.\n"
            , to_string().c_str()
            , (buffer_size-buffer_offset), buffer);
        return net::epoll_close;
    }
#endif
    
    try
    {
        // 接收数据
        retval = receive(buffer+buffer_offset, buffer_size-buffer_offset);
        if (0 == retval)
        {
            SERVER_LOG_DEBUG("Waiter %s closed by peer.\n", to_string().c_str());
            return net::epoll_close;
        }
        if (-1 == retval)
        {
            return net::epoll_none;
        }
    }
    catch (sys::CSyscallException& ex)
    {
        // EINVAL比较容易犯，故特别处理一下，以方便定位问题
        if (EINVAL == ex.get_errcode())
        {
            SERVER_LOG_ERROR("Invalid argument: %s.\n", request_context->to_string().c_str());
        }
        else
        {
            SERVER_LOG_DEBUG("Receive argument: %s.\n", request_context->to_string().c_str());
        }

        throw;
    }
     
#if 0
    SERVER_LOG_DEBUG("[%s] %u:%.*s.\n"
                    , to_string().c_str()                    
                    , (uint32_t)retval
                    , (int)retval
                    , buffer+buffer_offset);
#endif
        
    // 处理收到的数据
    Indicator indicator;
    indicator.reset = false;
    indicator.thread_index = get_thread_index();
    indicator.epoll_events = EPOLLOUT;

    utils::handle_result_t handle_result = _packet_handler->on_handle_request((size_t)retval, indicator);
    if (indicator.reset)
    {
        reset();
    }
    if (utils::handle_release == handle_result)
    {
        // 释放对Waiter的控制权
        HandOverParam* handover_param = static_cast<HandOverParam*>(ouput_ptr);
        handover_param->thread_index = indicator.thread_index;
        handover_param->epoll_events = indicator.epoll_events;
        return net::epoll_release;
    }
    else if (utils::handle_finish == handle_result)
    {
        // 将do_handle_epoll_send改成epoll_read_write，结构相对统一，但性能稍有下降
        //return do_handle_epoll_send(ptr);
        return net::epoll_write;
    }
    else if (util::handle_continue == handle_result)
    {        
        //SERVER_LOG_DEBUG("%s continue to receive ...\n", to_string().c_str());
        return net::epoll_none; // 也可以返回net::epoll_read
    }
    else
    {
        SERVER_LOG_DEBUG("%s protocol parse error.\n", to_string().c_str());
        return net::epoll_close;
    }    
}

net::epoll_event_t CWaiter::do_handle_epoll_error(void* input_ptr, void* ouput_ptr)
{
    SERVER_LOG_DEBUG("%s: %s.\n", to_string().c_str(), (char*)input_ptr);
    return net::epoll_close;
}

std::string CWaiter::str() const
{
    return to_string();
}

net::port_t CWaiter::self_port() const
{
    return get_self_port();
}

net::port_t CWaiter::peer_port() const
{
    return get_peer_port();
}

const net::ip_address_t& CWaiter::self_ip() const
{
    return get_self_ip();
}

const net::ip_address_t& CWaiter::peer_ip() const
{
    return get_peer_ip();
}

uint16_t CWaiter::get_thread_index() const
{
    return _thread_index;
}

SERVER_NAMESPACE_END
