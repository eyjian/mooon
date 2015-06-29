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
#ifndef MOOON_AGENT_THREAD_H
#define MOOON_AGENT_THREAD_H
#include <list>
#include <mooon/net/epoller.h>
#include <mooon/sys/lock.h>
#include <mooon/sys/thread.h>
#include <mooon/agent/agent.h>
#include "agent_connector.h"
#include "center_host.h"
#include "processor_manager.h"
#include "report_queue.h"
AGENT_NAMESPACE_BEGIN

class CAgentContext;
class CAgentThread: public sys::CThread
{
public:
    CAgentThread(CAgentContext* context);
    ~CAgentThread();
    
    bool put_message(const net::TCommonMessageHeader* header, uint32_t timeout_millisecond);
    const net::TCommonMessageHeader* get_message();
    void enable_queue_read();
    void enable_connector_write();
    bool register_command_processor(ICommandProcessor* processor);
    void deregister_command_processor(ICommandProcessor* processor);
    void set_center(const std::string& domainname_or_iplist, uint16_t port);
    
    CProcessorManager* get_processor_manager()
    {
        return &_processor_manager;
    }
     
private:
    virtual void run();
    virtual bool before_start();
    virtual void before_stop();
    
private:    
    bool parse_domainname_or_iplist();
    void clear_center_hosts();
    CCenterHost* choose_center_host();
    CCenterHost* poll_choose_center_host();
    void send_heartbeat();
    bool connect_center();
    std::string wait_domainname_or_iplist_ready(uint16_t* port);

private:
    TAgentInfo _agent_info;
    CAgentContext* _context;
    net::CEpoller _epoller;
    sys::CLock _queue_lock;
    CAgentConnector _connector;
    CReportQueue _report_queue;
    CProcessorManager _processor_manager;
    
private:
    sys::CEvent _center_event;
    sys::CLock _center_lock;
    std::string _domainname_or_iplist;
    uint16_t _port;
    std::list<CCenterHost*> _center_hosts;
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_THREAD_H
