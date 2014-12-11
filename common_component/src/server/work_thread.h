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
#ifndef MOOON_SERVER_THREAD_H
#define MOOON_SERVER_THREAD_H
#include <net/epoller.h>
#include <sys/pool_thread.h>
#include <util/timeout_manager.h>
#include "log.h"
#include "listener.h"
#include "waiter_pool.h"
SERVER_NAMESPACE_BEGIN

// CWaiter切换线程参数
struct HandOverParam
{
    uint32_t epoll_events; // 注入到新线程中的Epoll事件
    uint16_t thread_index; // 新的线程顺序号

    HandOverParam(uint16_t index)
     :epoll_events(EPOLLIN)
     ,thread_index(index)
    {
    }
};

class CContext;
class CWorkThread: public sys::CPoolThread
                 , public util::ITimeoutHandler<CWaiter>
{
public:
    CWorkThread();
	~CWorkThread();    

    void del_waiter(CWaiter* waiter);       
    void remove_waiter(CWaiter* waiter);       
    void update_waiter(CWaiter* waiter);  
    bool add_waiter(int fd, const net::ip_address_t& peer_ip, net::port_t peer_port
                          , const net::ip_address_t& self_ip, net::port_t self_port);   
      
    void add_listener_array(CListener* listener_array, uint16_t listen_count);    
    bool takeover_waiter(CWaiter* waiter, uint32_t epoll_event);
        
private:
    virtual void run();
    virtual bool before_run();
    virtual void after_run();
    virtual bool before_start(); 
    virtual void before_stop();
    virtual void on_timeout_event(CWaiter* waiter);
    virtual uint16_t index() const;    

public:
    virtual void set_parameter(void* parameter);

private:    
    void check_pending_queue();
    bool watch_waiter(CWaiter* waiter, uint32_t epoll_events);
    void handover_waiter(CWaiter* waiter, const HandOverParam& handover_param);

private:    
    time_t _current_time;
    net::CEpoller _epoller;
    CWaiterPool* _waiter_pool;       
    util::CTimeoutManager<CWaiter> _timeout_manager;    
    CContext* _context;
    IThreadFollower* _follower;
    
private:    
    struct PendingInfo
    {
        CWaiter* waiter;
        uint32_t epoll_events;
        PendingInfo(CWaiter* waiter_, uint32_t epoll_events_)
            :waiter(waiter_)
            ,epoll_events(epoll_events_)
        {
        }
    };
    sys::CLock _pending_lock;
    util::CArrayQueue<PendingInfo*>* _takeover_waiter_queue;
    
private:
    typedef void (CWorkThread::*epoll_event_proc_t)(net::CEpollable* epollable, void* param);
    epoll_event_proc_t _epoll_event_proc[8];

    void init_epoll_event_proc();
    void epoll_event_none(net::CEpollable* epollable, void* param);
    void epoll_event_read(net::CEpollable* epollable, void* param);
    void epoll_event_write(net::CEpollable* epollable, void* param);
    void epoll_event_readwrite(net::CEpollable* epollable, void* param);
    void epoll_event_remove(net::CEpollable* epollable, void* param);
    void epoll_event_close(net::CEpollable* epollable, void* param);
    void epoll_event_destroy(net::CEpollable* epollable, void* param);
    void epoll_event_release(net::CEpollable* epollable, void* param);  
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_THREAD_H
