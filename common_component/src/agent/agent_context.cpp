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
#include "agent_context.h"
#include <stdarg.h>
#include <util/string_util.h>
AGENT_NAMESPACE_BEGIN

sys::ILogger* logger = NULL;

IAgent* create(const TAgentInfo& agent_info)
{
    CAgentContext* agent = NULL;
    
    try
    {
        agent = new CAgentContext(agent_info);
        if (!agent->create())
        {
            throw sys::CSyscallException(EINVAL, __FILE__, __LINE__, "create agent");
        }
        
        AGENT_LOG_INFO("Created agent successfully.\n");
    }
    catch (sys::CSyscallException& ex)
    {
        AGENT_LOG_ERROR("Created agent error: %s.\n", ex.to_string().c_str());
        delete agent;
        agent = NULL;
    }
    
    return agent;
}

void destroy(IAgent* agent)
{
    if (agent != NULL)
    {
        AGENT_LOG_INFO("To destroy agent ...\n");
        CAgentContext* agent_impl = static_cast<CAgentContext*>(agent);
        agent_impl->destroy();
        AGENT_LOG_INFO("Agent was destroyed now.\n");
        delete agent_impl;        
    }
}

////////////////////////////////////////////////////////////
CAgentContext::CAgentContext(const TAgentInfo& agent_info)
 :_agent_info(agent_info)
{
    _agent_thread = new CAgentThread(this);
}

CAgentContext::~CAgentContext()
{    
    delete _agent_thread;
    delete _agent_info.heartbeat_hook;
}

bool CAgentContext::create()
{
    _agent_thread->start();    
    return true;
}

void CAgentContext::destroy()
{
    _agent_thread->stop();
}

void CAgentContext::set_center(const std::string& domainname_or_iplist, uint16_t port)
{
    _agent_thread->set_center(domainname_or_iplist, port);
}

bool CAgentContext::report(const char* data, size_t data_size, uint32_t timeout_millisecond)
{
	char* buffer = new char[data_size + sizeof(net::TCommonMessageHeader)];
    report_message_t* report_message = reinterpret_cast<report_message_t*>(buffer);
    
    report_message->header.size = data_size;
    report_message->header.command = U_REPORT_MESSAGE;
    memcpy(report_message->data, data, data_size);
    
    if (_agent_thread->put_message(&report_message->header, timeout_millisecond))
    {
    	return true;
    }

    delete []buffer;
    return false;
}

bool CAgentContext::report(uint32_t timeout_millisecond, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char* data = new char[REPORT_MAX + sizeof(net::TCommonMessageHeader)];
	util::DeleteHelper<char> dh(data, true);

	int data_bytes = util::CStringUtil::fix_vsnprintf(data, REPORT_MAX, format, args);
	return report(data, data_bytes, timeout_millisecond);
}

bool CAgentContext::register_command_processor(ICommandProcessor* processor)
{
    return _agent_thread->register_command_processor(processor);
}

void CAgentContext::deregister_command_processor(ICommandProcessor* processor)
{
    _agent_thread->deregister_command_processor(processor);
}

AGENT_NAMESPACE_END
