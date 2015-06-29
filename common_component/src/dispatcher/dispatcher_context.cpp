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
#include <sstream>
#include <mooon/sys/utils.h>
#include "dispatcher_context.h"
DISPATCHER_NAMESPACE_BEGIN

CDispatcherContext::~CDispatcherContext()
{    
    destroy_thread_pool();
        
    // 要晚于线程被删除，因为在线程被停止前，可能仍在被使用
    delete _managed_sender_table;
    delete _unmanaged_sender_table;
}

CDispatcherContext::CDispatcherContext(uint16_t thread_count, uint32_t timeout_seconds)
    :_timeout_seconds(timeout_seconds)
    ,_thread_pool(NULL)
{    
	set_reconnect_seconds(2); // 默认重连接间隔秒数

    _thread_count = thread_count;
    if (_thread_count < 1)
        _thread_count = get_default_thread_count();   

    _managed_sender_table = new CManagedSenderTable(this);
    _unmanaged_sender_table = new CUnmanagedSenderTable(this);
}

bool CDispatcherContext::create()
{           
    return create_thread_pool();    
}

void CDispatcherContext::add_sender(CSender* sender)
{
    CSendThread* send_thread = _thread_pool->get_next_thread();
    sender->inc_refcount();

    // 绑定到线程，并与线程建立关系联系
    sender->attach_thread(send_thread);
    send_thread->add_sender(sender);
}

IManagedSenderTable* CDispatcherContext::get_managed_sender_table()
{
    return _managed_sender_table;
}

IUnmanagedSenderTable* CDispatcherContext::get_unmanaged_sender_table()
{
    return _unmanaged_sender_table;
}

uint16_t CDispatcherContext::get_thread_number() const
{
    return _thread_pool->get_thread_count();
}

void CDispatcherContext::set_reconnect_seconds(uint32_t seconds)
{
	atomic_set(&_reconnect_seconds, seconds);
}

bool CDispatcherContext::create_thread_pool()
{        
    try
    {                                    
        // 创建线程池
        // 只有CThread::before_start返回false，create才会返回false
        _thread_pool = new CSendThreadPool;
        _thread_pool->create(_thread_count, this);
        DISPATCHER_LOG_INFO("Sender thread number is %d.\n", _thread_pool->get_thread_count());

        CSendThread** send_thread = _thread_pool->get_thread_array();
        uint16_t thread_count = _thread_pool->get_thread_count();
        for (uint16_t i=0; i<thread_count; ++i)
        {                        
            send_thread[i]->wakeup();
        }

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        delete _thread_pool;
        DISPATCHER_LOG_ERROR("Failed to create thread pool: %s.\n", ex.to_string().c_str());
    }

    return false;
}

void CDispatcherContext::destroy_thread_pool()
{
    if (_thread_pool != NULL)
    {
        _thread_pool->destroy();
        delete _thread_pool;
        _thread_pool = NULL;
    }
}

uint16_t CDispatcherContext::get_default_thread_count() const
{
    // 设置默认的线程池中线程个数为CPU核个数减1个，如果取不到CPU核个数，则取1
    uint16_t thread_count = sys::CUtils::get_cpu_number();
    return (thread_count < 2)? 1: thread_count-1;
}

//////////////////////////////////////////////////////////////////////////
// 模块日志器
sys::ILogger* logger = NULL;

void destroy(IDispatcher* dispatcher)
{
    delete dispatcher;
}

IDispatcher* create(uint16_t thread_count, uint32_t timeout_seconds)
{    
    CDispatcherContext* dispatcher = new CDispatcherContext(thread_count, timeout_seconds);    
    if (!dispatcher->create())
    {
        delete dispatcher;
        dispatcher = NULL;
    }

    return dispatcher;
}

std::string sender_info_tostring(const SenderInfo& send_info)
{
    std::stringstream str;
    str << "send_info://"
        << send_info.key
        << "@"
        << send_info.ip_node.ip.to_string()
        << ":"
        << send_info.ip_node.port
        << "-"
        << send_info.queue_size
        << "-"
        << send_info.resend_times
        << "-"
        << send_info.reconnect_times
        << "-"
        << send_info.reply_handler;

    return str.str();
}

DISPATCHER_NAMESPACE_END
