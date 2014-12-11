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
#ifndef MOOON_DISPATCHER_SEND_THREAD_H
#define MOOON_DISPATCHER_SEND_THREAD_H
#include <list>
#include <net/epoller.h>
#include <sys/pool_thread.h>
#include <util/timeout_manager.h>
#include "dispatcher_log.h"
#include "dispatcher/dispatcher.h"
DISPATCHER_NAMESPACE_BEGIN

class CSender;
class CDispatcherContext;
class CSendThread: public sys::CPoolThread, public util::ITimeoutHandler<CSender>
{
    typedef std::list<CSender*> CSenderQueue;
    
public:
    CSendThread();
    time_t get_current_time() const;
    void add_sender(CSender* sender);
    virtual void set_parameter(void* parameter);

    net::CEpoller& get_epoller() const { return _epoller; }
    util::CTimeoutManager<CSender>* get_timeout_manager() { return &_timeout_manager; }                
        
private:
    virtual void run();  
    virtual bool before_run();
    virtual void after_run();
    virtual bool before_start();   
    virtual void before_stop();
    virtual void on_timeout_event(CSender* timeoutable);
    
private:    
    void clear_timeout_queue();
    void clear_reconnect_queue();
    void clear_unconnected_queue();

private:
    void check_reconnect_queue(); // 处理_reconnect_queue
    void check_unconnected_queue(); // 处理_unconnected_queue
    void remove_sender(CSender* sender);
    void sender_connect(CSender* sender);
    void sender_reconnect(CSender* sender);
    
private:
    time_t _current_time;
    time_t _last_connect_time;   // 上一次连接时间    
    
private:
    typedef void (CSendThread::*epoll_event_proc_t)(net::CEpollable* epollable);
    epoll_event_proc_t _epoll_event_proc[8];

    void init_epoll_event_proc();
    void epoll_event_none(net::CEpollable* epollable);
    void epoll_event_read(net::CEpollable* epollable);
    void epoll_event_write(net::CEpollable* epollable);
    void epoll_event_readwrite(net::CEpollable* epollable);
    void epoll_event_remove(net::CEpollable* epollable);
    void epoll_event_close(net::CEpollable* epollable);
    void epoll_event_destroy(net::CEpollable* epollable);
    void epoll_event_release(net::CEpollable* epollable);    

private:     
    mutable net::CEpoller _epoller;
    sys::CLock _unconnected_lock;
    CSenderQueue _reconnect_queue; // 重连接队列
    CSenderQueue _unconnected_queue; // 待连接队列    
    CDispatcherContext* _context;
    util::CTimeoutManager<CSender> _timeout_manager;
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_SEND_THREAD_H
