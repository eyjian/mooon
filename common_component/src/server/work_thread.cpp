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
 * Author: jian yi, eyjian@qq.com
 */
#include <sstream>
#include <mooon/net/utils.h>
#include <mooon/sys/utils.h>
#include "context.h"
#include "work_thread.h"
SERVER_NAMESPACE_BEGIN

CWorkThread::CWorkThread()
    :_waiter_pool(NULL)
    ,_context(NULL)
    ,_follower(NULL)
    ,_takeover_waiter_queue(NULL)
{
    _current_time = time(NULL);
    _timeout_manager.set_timeout_handler(this);  

    init_epoll_event_proc();     
}

CWorkThread::~CWorkThread()
{
    _epoller.destroy();
    delete _follower;
    delete _takeover_waiter_queue;
}

void CWorkThread::run()
{
    int retval; // _epoller.timed_wait的返回值

    _timeout_manager.check_timeout(_current_time);
    check_pending_queue();
        
    // epoll前回调
    if (_follower != NULL)
    {
        _follower->before_epoll();
    }
    try
    {                
        // EPOLL检测
        retval = _epoller.timed_wait(_context->get_config()->get_epoll_timeout_milliseconds());        
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_FATAL("Waiter thread wait error for %s.\n", ex.to_string().c_str());
        throw; // timed_wait异常是不能恢复的
    }

    try
    {        
        // 得到当前时间
        _current_time = time(NULL);

        if (0 == retval) // timeout
        {
            // epoll后回调: 超时
            if (_follower != NULL)
            {
                _follower->after_epoll(_current_time, true);
            }

            return; // 直接返回，再次进入epoll等待
        }
    
        // epoll后回调: 网络事件
        if (_follower != NULL)
        {
            _follower->after_epoll(_current_time, false);
        }
        for (int i=0; i<retval; ++i)
        {            
            HandOverParam handover_param(this->get_index());
            net::CEpollable* epollable = _epoller.get(i);
            net::epoll_event_t retval = epollable->handle_epoll_event(this, _epoller.get_events(i), &handover_param);

            if ((retval < 8) && (retval >= 0))
            {
                (this->*_epoll_event_proc[retval])(epollable, &handover_param);
            }
            else
            {
                epoll_event_close(epollable, NULL);
            }            
        }
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_FATAL("Waiter thread run error for %s.\n", ex.to_string().c_str());
    }
}

bool CWorkThread::before_run()
{
#if ENABLE_SET_SERVER_THREAD_NAME==1
    std::stringstream thread_name;
    thread_name << "svr-thread[" << get_index() << "]";
    sys::CUtils::set_process_name(thread_name.str().c_str());
#endif // ENABLE_SET_SERVER_THREAD_NAME

    if (NULL == _follower) return true;
    return _follower->before_run();
}

void CWorkThread::after_run()
{
    if (_follower != NULL)
        _follower->after_run();
    SERVER_LOG_INFO("Server thread %u has exited.\n", get_thread_id());
}

bool CWorkThread::before_start()
{
    try
    {      
        IConfig* config = _context->get_config();
        IFactory* factory = _context->get_factory();

        _follower = factory->create_thread_follower(get_index());
        _takeover_waiter_queue = new utils::CArrayQueue<PendingInfo*>(config->get_takeover_queue_size());
        _timeout_manager.set_timeout_seconds(config->get_connection_timeout_seconds());       
        
        _epoller.create(config->get_epoll_size());        
        
        uint32_t thread_connection_pool_size = config->get_connection_pool_size();
        
        try
        {
            _waiter_pool = new CWaiterPool(this, factory, thread_connection_pool_size);
        }
        catch (std::runtime_error& ex)
        {
            SERVER_LOG_ERROR("%s.\n", ex.what());
            return false;
        }

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_ERROR("Start server-thread error: %s.\n", ex.to_string().c_str());
        return false;
    }
}

void CWorkThread::before_stop()
{
    _epoller.wakeup();
}

void CWorkThread::set_parameter(void* parameter)
{
    _context = static_cast<CContext*>(parameter);
}

void CWorkThread::on_timeout_event(CWaiter* waiter)
{
    SERVER_LOG_DEBUG("%s is timeout.\n", waiter->to_string().c_str());

    if (!waiter->on_timeout())
    {
        _timeout_manager.update(waiter, _current_time);
    }
    else
    {
        try
        {
            _epoller.del_events(waiter);                    
        }
        catch (sys::CSyscallException& ex)
        {
            SERVER_LOG_ERROR("Deleted %s error: %s.\n", waiter->to_string().c_str(), ex.to_string().c_str());
        }
        
        _waiter_pool->push_waiter(waiter);
    }
}

uint16_t CWorkThread::index() const
{
    return get_index();
}

bool CWorkThread::takeover_waiter(CWaiter* waiter, uint32_t epoll_event)
{
    if (_takeover_waiter_queue->is_full()) return false;
    
    sys::LockHelper<sys::CLock> lock_helper(_pending_lock);
    if (_takeover_waiter_queue->is_full()) return false;
    
    PendingInfo* pending_info = new PendingInfo(waiter, epoll_event);
    _takeover_waiter_queue->push_back(pending_info);
    _epoller.wakeup();
    
    return true;
}

void CWorkThread::check_pending_queue()
{
    if (!_takeover_waiter_queue->is_empty())
    {
        sys::LockHelper<sys::CLock> lock_helper(_pending_lock);
        while (!_takeover_waiter_queue->is_empty())
        {
            PendingInfo* pending_info = _takeover_waiter_queue->pop_front();
            pending_info->waiter->set_thread_index(get_index());
            watch_waiter(pending_info->waiter, pending_info->epoll_events);
            delete pending_info;
        }
    }
}

bool CWorkThread::watch_waiter(CWaiter* waiter, uint32_t epoll_events)
{
    try
    {               
        _epoller.set_events(waiter, epoll_events);
        _timeout_manager.push(waiter, _current_time);

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        _waiter_pool->push_waiter(waiter);
        SERVER_LOG_ERROR("Set %s epoll events error: %s.\n"
            , waiter->to_string().c_str()
            , ex.to_string().c_str());
        
        return false;
    }
}

void CWorkThread::handover_waiter(CWaiter* waiter, const HandOverParam& handover_param)
{
    std::string waiter_str = waiter->to_string();
    CWorkThread* takeover_thread = _context->get_thread(handover_param.thread_index);
    if (NULL == takeover_thread)
    {
        SERVER_LOG_ERROR("No thread[%u] to take over %s.\n", handover_param.thread_index, waiter_str.c_str());
        waiter->on_switch_failure(false);
        _waiter_pool->push_waiter(waiter);
    }
    else if (takeover_thread->takeover_waiter(waiter, handover_param.epoll_events))
    {
        SERVER_LOG_DEBUG("Handover %s from thread[%u] to thread[%u].\n", waiter_str.c_str(), get_index(), handover_param.thread_index);
    }
    else
    {
        SERVER_LOG_ERROR("Can not handover %s from thread[%u] to thread[%u].\n", waiter_str.c_str(), get_index(), handover_param.thread_index);
        waiter->on_switch_failure(true);
        _waiter_pool->push_waiter(waiter);
    }
}

void CWorkThread::del_waiter(CWaiter* waiter)
{    
    remove_waiter(waiter);
    _waiter_pool->push_waiter(waiter);
}

void CWorkThread::remove_waiter(CWaiter* waiter)
{
    try
    {
        _epoller.del_events(waiter);        
        _timeout_manager.remove(waiter);        
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_ERROR("Delete waiter error for %s.\n", ex.to_string().c_str());
    }    
}

void CWorkThread::update_waiter(CWaiter* waiter)
{
    _timeout_manager.remove(waiter);
    _timeout_manager.push(waiter, _current_time);
}

bool CWorkThread::add_waiter(int fd, const net::ip_address_t& peer_ip, net::port_t peer_port
                                     , const net::ip_address_t& self_ip, net::port_t self_port)
{
    CWaiter* waiter = _waiter_pool->pop_waiter();
    if (NULL == waiter)
    {
        SERVER_LOG_WARN("Waiter overflow - %s:%d.\n", peer_ip.to_string().c_str(), peer_port);
        return false;
    }    
        
    waiter->attach(fd, peer_ip, peer_port);
    waiter->set_nonblock(true); // 设置为非阻塞
    waiter->set_self(self_ip, self_port);
    return watch_waiter(waiter, EPOLLIN);    
}

void CWorkThread::add_listener_array(CListener* listener_array, uint16_t listen_count)
{        
    for (uint16_t i=0; i<listen_count; ++i)
         _epoller.set_events(&listener_array[i], EPOLLIN, true);    
}

void CWorkThread::init_epoll_event_proc()
{
    using namespace net;
    _epoll_event_proc[epoll_none/*0*/]       = &CWorkThread::epoll_event_none;
    _epoll_event_proc[epoll_read/*1*/]       = &CWorkThread::epoll_event_read;
    _epoll_event_proc[epoll_write/*2*/]      = &CWorkThread::epoll_event_write;
    _epoll_event_proc[epoll_read_write/*3*/] = &CWorkThread::epoll_event_readwrite;
    _epoll_event_proc[epoll_close/*4*/]      = &CWorkThread::epoll_event_close;
    _epoll_event_proc[epoll_remove/*5*/]     = &CWorkThread::epoll_event_remove;
    _epoll_event_proc[epoll_destroy/*6*/]    = &CWorkThread::epoll_event_destroy;
    _epoll_event_proc[epoll_release/*7*/]    = &CWorkThread::epoll_event_release;
}

void CWorkThread::epoll_event_none(net::CEpollable* epollable, void* param)
{
    HandOverParam* handover_param = static_cast<HandOverParam*>(param);
    _epoller.set_events(epollable, handover_param->epoll_events);
}

void CWorkThread::epoll_event_read(net::CEpollable* epollable, void* param)
{
    _epoller.set_events(epollable, EPOLLIN);
}

void CWorkThread::epoll_event_write(net::CEpollable* epollable, void* param)
{
    _epoller.set_events(epollable, EPOLLOUT);
}

void CWorkThread::epoll_event_readwrite(net::CEpollable* epollable, void* param)
{
    _epoller.set_events(epollable, EPOLLIN|EPOLLOUT);
}

void CWorkThread::epoll_event_remove(net::CEpollable* epollable, void* param)
{
    epoll_event_close(epollable, param);
}

void CWorkThread::epoll_event_close(net::CEpollable* epollable, void* param)
{
    del_waiter((CWaiter*)epollable);
}

void CWorkThread::epoll_event_destroy(net::CEpollable* epollable, void* param)
{
    epoll_event_close(epollable, param);
}

void CWorkThread::epoll_event_release(net::CEpollable* epollable, void* param)
{
    CWaiter* waiter = static_cast<CWaiter*>(epollable);

    if (_waiter_pool->is_valid())
    {
        SERVER_LOG_WARN("Can not handover %s.\n", waiter->to_string().c_str());
        del_waiter(waiter);
    }
    else
    {                        
        HandOverParam* handover_param = static_cast<HandOverParam*>(param);
        if (handover_param->thread_index == get_index())
        {
            // 同一线程，只做epoll事件的变更
            _epoller.set_events(epollable, handover_param->epoll_events);
        }
        else
        {
            // 从epoll中移除连接，但不关闭连接
            remove_waiter(waiter);
            handover_waiter(waiter, *handover_param);
        }
    }
}

SERVER_NAMESPACE_END
