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
#include "agent_thread.h"
#include <algorithm>
#include <util/token_list.h>
#include "agent_context.h"
AGENT_NAMESPACE_BEGIN

CAgentThread::CAgentThread(CAgentContext* context)
 :_context(context)
 ,_connector(this)
 ,_report_queue(context->get_agent_info().queue_size + 1, this)
{
    _connector.set_connect_timeout_milliseconds(
            context->get_agent_info().connect_timeout_milliseconds);
}

CAgentThread::~CAgentThread()
{
    clear_center_hosts();
}

bool CAgentThread::put_message(const net::TCommonMessageHeader* header, uint32_t timeout_millisecond)
{
    sys::LockHelper<sys::CLock> lh(_queue_lock);
    return _report_queue.push_back(const_cast<net::TCommonMessageHeader*>(header), timeout_millisecond);
}

const net::TCommonMessageHeader* CAgentThread::get_message()
{
    net::TCommonMessageHeader* agent_message = NULL;
    sys::LockHelper<sys::CLock> lh(_queue_lock);
    
    _report_queue.pop_front(agent_message);
    return agent_message;
}

void CAgentThread::enable_queue_read()
{    
    _epoller.set_events(&_report_queue, EPOLLIN);
}

void CAgentThread::enable_connector_write()
{
    _epoller.set_events(&_connector, EPOLLIN | EPOLLOUT);
}

bool CAgentThread::register_command_processor(ICommandProcessor* processor)
{
    return _processor_manager.register_processor(processor);
}

void CAgentThread::deregister_command_processor(ICommandProcessor* processor)
{
    _processor_manager.deregister_processor(processor);
}

void CAgentThread::set_center(const std::string& domainname_or_iplist, uint16_t port)
{
    AGENT_LOG_DEBUG("set center[%s:%u].\n", domainname_or_iplist.c_str(), port);
    
    sys::LockHelper<sys::CLock> lh(_center_lock);
    _domainname_or_iplist = domainname_or_iplist;
    _port = port;
    
    _center_event.signal();
}

void CAgentThread::run()
{
    AGENT_LOG_INFO("Agent thread ID is %u.\n", get_thread_id());
    
    while (true)
    {
        try
        {
            // 必须先建立连接
            while (!is_stop() 
                && !_connector.is_connect_established())
            {
                if (connect_center())
                    break;
            }
            if (is_stop())
            {
                AGENT_LOG_INFO("Thread[%u] is tell to stop.\n", get_thread_id());
                break;
            }
                        
            int num = _epoller.timed_wait(_connector.get_connect_timeout_milliseconds());
            if (0 == num)
            {
                // timeout to send heartbeat
                AGENT_LOG_DEBUG("Agent timeout to send heartbeat.\n");
                send_heartbeat();
            }
            else
            {
                for (int i=0; i<num; ++i)
                {
                    uint32_t events = _epoller.get_events(i);
                    net::CEpollable* epollable = _epoller.get(i); 
                    
                    net::epoll_event_t ee = epollable->handle_epoll_event(NULL, events, NULL);
                    switch (ee)
                    {
                    case net::epoll_read:
                        _epoller.set_events(epollable, EPOLLIN);
                        break;
                    case net::epoll_write:
                        _epoller.set_events(epollable, EPOLLOUT);
                        break;
                    case net::epoll_read_write:
                        _epoller.set_events(epollable, EPOLLIN | EPOLLOUT);
                        break;
                    case net::epoll_remove:
                        _epoller.del_events(epollable);
                        break;
                    case net::epoll_close:
                        _epoller.del_events(epollable);
                        epollable->close();
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        catch (sys::CSyscallException& ex)
        {
            AGENT_LOG_ERROR("Network exception: %s.\n", ex.to_string().c_str());
        }
    }
    
    _epoller.destroy();
    AGENT_LOG_INFO("Agent thread[%u] exited.\n", get_thread_id());
}

bool CAgentThread::before_start()
{
    _epoller.create(1024);
    enable_queue_read();
    
    return true;
}

void CAgentThread::before_stop()
{
    sys::LockHelper<sys::CLock> lh(_center_lock);
    _center_event.signal();
}

// 原则：
// 如果解析出新的IP，则如果上一次解析出来的，但在新的列表中未出现的IP，
// 需要被删除，而仍存在的则保持，并且状态不变；
// 如果没有解析出新出现的IP，则保持不变；
// 如果新的解析没有IP，则使用上一次解析出的IP
//
// 注：因为IP个数通常在两三个内，所以不用考虑查找性能，只求简单
bool CAgentThread::parse_domainname_or_iplist()
{    
    uint16_t port;
    std::string domainname_or_iplist;
    
    // 如果还没有设置好域名或IP列表，则等待
    domainname_or_iplist = wait_domainname_or_iplist_ready(&port);
    if (is_stop())
    {
        // 也许在等待过程中，线程收到了退出指令
        return false;
    }
    
    std::string errinfo;
    net::string_ip_array_t string_ip_array;
    if (!net::CUtil::get_ip_address(domainname_or_iplist.c_str(), string_ip_array, errinfo))
    {
        // 也许是IP列表，尝试一下
        util::CTokenList::TTokenList token_list;
        util::CTokenList::parse(token_list, domainname_or_iplist, ",");
        if (token_list.empty())
        {
            AGENT_LOG_WARN("Not found any IP from %s.\n", domainname_or_iplist.c_str());
            return false;
        }
        
        std::copy(token_list.begin(), token_list.end(), std::back_inserter(string_ip_array));
    }

    // 如果新解析出IP，否则保持不变，也就是什么也不用做
    if (!string_ip_array.empty())
    {
    	CCenterHost* center_host = NULL;
    	net::string_ip_array_t::iterator ip_iter;
    	std::list<CCenterHost*>::iterator hosts_iter;

    	// 需要将没有再出现的IP删除掉
    	for (hosts_iter = _center_hosts.begin(); hosts_iter != _center_hosts.end(); ++hosts_iter)
    	{
    		center_host = *hosts_iter;
    		ip_iter = std::find(string_ip_array.begin(), string_ip_array.end(), center_host->get_ip());

    		if (ip_iter == string_ip_array.end())
    		{
    			// 没有再出现
    			AGENT_LOG_INFO("Remove center[%s].\n", center_host->get_ip().c_str());
    			hosts_iter = _center_hosts.erase(hosts_iter);
    			delete center_host;
    		}
    	}

    	// 将新出现的添加进来
    	ip_iter = string_ip_array.begin();
    	for ( ; ip_iter != string_ip_array.end(); ++ip_iter)
    	{
    		hosts_iter = std::find(_center_hosts.begin(), _center_hosts.end(), *ip_iter);
    		if (hosts_iter == _center_hosts.end())
    		{
    			center_host = new CCenterHost(*ip_iter, port);
    			_center_hosts.push_back(center_host);

    			AGENT_LOG_INFO("Center[%s] added.\n", (*ip_iter).c_str());
    		}
    	}
    }
    
    return !_center_hosts.empty();
}

void CAgentThread::clear_center_hosts()
{    
    for (std::list< CCenterHost*>::iterator iter = _center_hosts.begin()
        ;iter != _center_hosts.end()
        ;++iter)
    {
        CCenterHost* center_host = *iter;
        delete center_host;
    }
       
    _center_hosts.clear();
}

// 根据参数决定使用哪种策略
CCenterHost* CAgentThread::choose_center_host()
{
	return poll_choose_center_host();
}

// 简单策略：轮流使用
CCenterHost* CAgentThread::poll_choose_center_host()
{
	CCenterHost* chosen_host = _center_hosts.front();

	_center_hosts.pop_front();
	_center_hosts.push_back(chosen_host);

	return chosen_host;
}

void CAgentThread::send_heartbeat()
{
    const TAgentInfo& agent_info = _context->get_agent_info();
    IHeartbeatHook* heartbeat_hook = agent_info.heartbeat_hook;

    size_t buffer_size = 0;
    if (NULL == heartbeat_hook)
    {
        buffer_size = sizeof(TSimpleHeartbeatMessage);
    }
    else
    {
        buffer_size = sizeof(TSimpleHeartbeatMessage) + heartbeat_hook->get_data_size();
    }
    
    char* heartbeat_buffer = new char[buffer_size];
    TSimpleHeartbeatMessage* heartbeat = reinterpret_cast<TSimpleHeartbeatMessage*>(heartbeat_buffer);

    heartbeat->header.size = buffer_size - sizeof(TSimpleHeartbeatMessage);
    heartbeat->header.command = U_SIMPLE_HEARTBEAT_MESSAGE;
    if (heartbeat->header.size > 0)
    {
        memcpy(heartbeat->app_data, heartbeat_hook->get_data(), heartbeat_hook->get_data_size());
    }

    put_message(&heartbeat->header, _connector.get_connect_timeout_milliseconds());
}

bool CAgentThread::connect_center()
{
    if (parse_domainname_or_iplist())
    {
        CCenterHost* host = choose_center_host();
        if (NULL == host)
        {
            AGENT_LOG_FATAL("No hosts chosen.\n");
            return false;
        }

        _connector.set_peer_ip(host->get_ip().c_str());
        _connector.set_peer_port(host->get_port());
        
        try
        {
            _connector.timed_connect();
            enable_connector_write();
            host->reset_reconn_times();
            AGENT_LOG_DEBUG("%s successfully.\n", _connector.to_string().c_str());   
            return true;
        }
        catch (sys::CSyscallException& ex)
        {
            host->inc_reconn_times();
            AGENT_LOG_ERROR("%s failed: %s.\n", _connector.to_string().c_str(), ex.to_string().c_str());
            do_millisleep(_connector.get_connect_timeout_milliseconds());
        }
    }
    
    return false;
}

std::string CAgentThread::wait_domainname_or_iplist_ready(uint16_t* port)
{
	std::string domainname_or_iplist;

    // 如果_domainname_or_iplist为空，则一直等待，直到不为空，或线程收到了退出指令
    while (!is_stop())
    {
        sys::LockHelper<sys::CLock> lh(_center_lock);
        domainname_or_iplist = _domainname_or_iplist;
        *port = _port;
        
        if (!domainname_or_iplist.empty())
        {
            break;
        }
        
        AGENT_LOG_INFO("Waiting for domain name or IP not set.\n");
        _center_event.wait(_center_lock);        
    }

    return domainname_or_iplist;
}

AGENT_NAMESPACE_END
