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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_DISPATCHER_CONTEXT_H
#define MOOON_DISPATCHER_CONTEXT_H
#include "mooon/dispatcher/dispatcher.h"

#include <mooon/sys/atomic.h>
#include <mooon/sys/lock.h>
#include <mooon/sys/read_write_lock.h>
#include <mooon/sys/thread_pool.h>

#include "send_thread.h"
#include "dispatcher_log.h"
#include "managed_sender_table.h"
#include "default_reply_handler.h"
#include "unmanaged_sender_table.h"
DISPATCHER_NAMESPACE_BEGIN

class CDispatcherContext: public IDispatcher
{
public:
    ~CDispatcherContext();
    CDispatcherContext(uint16_t thread_count, uint32_t timeout_seconds);
    
    bool create();         
    void add_sender(CSender* sender); 

    uint32_t get_timeout_seconds() const
    {
    	return _timeout_seconds;
    }

    uint32_t get_reconnect_seconds() const
    {
    	return static_cast<uint32_t>(atomic_read(&_reconnect_seconds));
    }

private: // IDispatcher
    virtual IManagedSenderTable* get_managed_sender_table();
    virtual IUnmanagedSenderTable* get_unmanaged_sender_table();
    virtual uint16_t get_thread_number() const;
    virtual void set_reconnect_seconds(uint32_t seconds);

private:        
    bool create_thread_pool();  
    void destroy_thread_pool();
    uint16_t get_default_thread_count() const;    

private:
    typedef sys::CThreadPool<CSendThread> CSendThreadPool;
    uint16_t _thread_count;
    uint32_t _timeout_seconds;
    atomic_t _reconnect_seconds;
    CSendThreadPool* _thread_pool;
    CManagedSenderTable* _managed_sender_table;
    CUnmanagedSenderTable* _unmanaged_sender_table;      
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_CONTEXT_H
