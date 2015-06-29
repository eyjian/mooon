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
#include <sstream>
#include <mooon/net/utils.h>
#include <mooon/sys/utils.h>
#include "send_thread.h"
#include "dispatcher_context.h"
#include "unmanaged_sender_table.h"
DISPATCHER_NAMESPACE_BEGIN

CSendThread::CSendThread()
    :_current_time(0)    
    ,_last_connect_time(0)
    ,_context(NULL)
{
    init_epoll_event_proc();
}

time_t CSendThread::get_current_time() const
{
    return _current_time;
}

void CSendThread::add_sender(CSender* sender)
{
	DISPATCHER_LOG_DEBUG("Thread[%u,%u] added %s.\n", get_index(), get_thread_id(), sender->to_string().c_str());
    sys::LockHelper<sys::CLock> lock_helper(_unconnected_lock);
    _unconnected_queue.push_back(sender);
    _epoller.wakeup();
}

void CSendThread::run()
{
    // 更新当前时间
    _current_time = time(NULL);
    
    // 调用check_reconnect_queue和check_unconnected_queue的顺序不要颠倒
    check_reconnect_queue();
    check_unconnected_queue();
    if (_timeout_manager.get_timeout_seconds() > 0)
    {
        _timeout_manager.check_timeout(_current_time);
    }

    int events_count = _epoller.timed_wait(2000);
    if (0 == events_count)
    {
        // 超时处理
    }
    else
    {
        for (int i=0; i<events_count; ++i)
        {
            net::CEpollable* epollable = _epoller.get(i);
            uint32_t events = _epoller.get_events(i);            

            net::epoll_event_t retval = epollable->handle_epoll_event(this, events, NULL);
            if ((retval < 8) && (retval >= 0))
            {
                (this->*_epoll_event_proc[retval])(epollable);
            }
            else
            {
                epoll_event_close(epollable);
            }
        }
    }
}

void CSendThread::init_epoll_event_proc()
{
    using namespace net;
    _epoll_event_proc[epoll_none/*0*/]       = &CSendThread::epoll_event_none;
    _epoll_event_proc[epoll_read/*1*/]       = &CSendThread::epoll_event_read;
    _epoll_event_proc[epoll_write/*2*/]      = &CSendThread::epoll_event_write;
    _epoll_event_proc[epoll_read_write/*3*/] = &CSendThread::epoll_event_readwrite;
    _epoll_event_proc[epoll_close/*4*/]      = &CSendThread::epoll_event_close;
    _epoll_event_proc[epoll_remove/*5*/]     = &CSendThread::epoll_event_remove;
    _epoll_event_proc[epoll_destroy/*6*/]    = &CSendThread::epoll_event_destroy;
    _epoll_event_proc[epoll_release/*7*/]    = &CSendThread::epoll_event_release;
}

void CSendThread::epoll_event_none(net::CEpollable* epollable)
{
    // do nothing
}

void CSendThread::epoll_event_read(net::CEpollable* epollable)
{
    _epoller.set_events(epollable, EPOLLIN);
}

void CSendThread::epoll_event_write(net::CEpollable* epollable)
{
    _epoller.set_events(epollable, EPOLLOUT);
}

void CSendThread::epoll_event_readwrite(net::CEpollable* epollable)
{
    _epoller.set_events(epollable, EPOLLIN|EPOLLOUT);
}

void CSendThread::epoll_event_remove(net::CEpollable* epollable)
{
    _epoller.del_events(epollable);
}

void CSendThread::epoll_event_close(net::CEpollable* epollable)
{
    sender_reconnect((CSender*)epollable); 
}

void CSendThread::epoll_event_destroy(net::CEpollable* epollable)
{
    remove_sender((CSender*)epollable);
}

void CSendThread::epoll_event_release(net::CEpollable* epollable)
{
    epoll_event_close((CSender*)epollable);
}

bool CSendThread::before_run() throw ()
{
#if ENABLE_SET_DISPATCHER_THREAD_NAME==1
    std::stringstream thread_name;
    thread_name << "snd-thread[" << get_index() << "]";
    sys::CUtils::set_process_name(thread_name.str().c_str());
#endif // ENABLE_SET_DISPATCHER_THREAD_NAME

    return true;
}

void CSendThread::after_run() throw ()
{    
    clear_unconnected_queue();
    clear_reconnect_queue();    
    clear_timeout_queue();

    DISPATCHER_LOG_INFO("Sending thread %u has exited.\n", get_thread_id());
}

void CSendThread::before_start() throw (utils::CException, sys::CSyscallException)
{
    _timeout_manager.set_timeout_seconds(_context->get_timeout_seconds());
    _timeout_manager.set_timeout_handler(this);    
    _epoller.create(10000);
}

void CSendThread::before_stop() throw (utils::CException, sys::CSyscallException)
{
    _epoller.wakeup();
}

void CSendThread::set_parameter(void* parameter)
{
    _context = static_cast<CDispatcherContext*>(parameter);
}

void CSendThread::on_timeout_event(CSender* timeoutable)
{
    if (timeoutable->on_timeout())
    {
        //remove_sender(timeoutable);
        sender_reconnect(timeoutable);
    }
    else
    {
        _timeout_manager.update(timeoutable, _current_time);
    }
}

void CSendThread::clear_timeout_queue()
{
    for (;;)
    {
        CSender* sender = _timeout_manager.pop_front();
        if (NULL == sender)
        {
            break;
        }

        remove_sender(sender);
    }
}

void CSendThread::clear_reconnect_queue()
{
    while (!_reconnect_queue.empty())
    {
        CSender* sender = _reconnect_queue.front();
        _reconnect_queue.pop_front();

        remove_sender(sender);
    }
}

void CSendThread::clear_unconnected_queue()
{
    while (!_unconnected_queue.empty())
    {
        CSender* sender = _unconnected_queue.front();
        _unconnected_queue.pop_front();
        
        remove_sender(sender);
    }
}

void CSendThread::check_reconnect_queue()
{
    // 限制重连接的频率
    if (_current_time - _last_connect_time < (time_t)_context->get_reconnect_seconds()) return;
    _last_connect_time = _current_time;
    
    CSenderQueue::size_type reconnect_number =  _reconnect_queue.size();
    for (CSenderQueue::size_type i=0; i<reconnect_number; ++i)
    {
        CSender* sender = _reconnect_queue.front();
        _reconnect_queue.pop_front();               

        // 如果最大重连接次数值为-1，说明总是重连接
        int max_reconnect_times = sender->get_sender_info().reconnect_times;
        if ((max_reconnect_times > -1)
         && (sender->get_reconnect_times() >= (uint32_t)max_reconnect_times))
        {
            remove_sender(sender);
            continue;
        }
        
        // 进行重连接
        sender_connect(sender);          
    }
}

void CSendThread::check_unconnected_queue()
{
    // 两个if可以降低do_connect对性能的影响
    if (_unconnected_queue.empty()) return;
    
    // 需要锁的保护
    sys::LockHelper<sys::CLock> lock_helper(_unconnected_lock);
    while (!_unconnected_queue.empty())
    {
        CSender* sender = _unconnected_queue.front();
        _unconnected_queue.pop_front();
        
        // 进行连接
        sender_connect(sender);
    }
}

void CSendThread::remove_sender(CSender* sender)
{    
    _epoller.del_events(sender);                
    _timeout_manager.remove(sender);

    CSenderTable* sender_table = sender->get_sender_table();
    sender_table->close_sender(sender);
}

void CSendThread::sender_connect(CSender* sender)
{
    try
    {
        // 必须采用异步连接，这个是性能的保证
        if (sender->async_connect())
        {
        	DISPATCHER_LOG_DEBUG("%s instantly connect successfully.\n", sender->to_string().c_str());
        }
        else
        {
        	DISPATCHER_LOG_DEBUG("%s to asynchronously connect.\n", sender->to_string().c_str());
        }

        _epoller.set_events(sender, EPOLLIN|EPOLLOUT);
        _timeout_manager.push(sender, _current_time);
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接未成功，再插入到队列尾部，由于有循环count次限制，所以放在尾部可以保证本轮不会再被处理
        sender_reconnect(sender);
        DISPATCHER_LOG_DEBUG("%s connected failed.\n", sender->to_string().c_str());                    
    }
}

void CSendThread::sender_reconnect(CSender* sender)
{
    sender->close();
    _epoller.del_events(sender);
    _timeout_manager.remove(sender);
    _reconnect_queue.push_back(sender);
}

DISPATCHER_NAMESPACE_END
