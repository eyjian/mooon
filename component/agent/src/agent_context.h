/*
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
#ifndef MOOON_AGENT_CONTEXT_H
#define MOOON_AGENT_CONTEXT_H
#include <mooon/agent/agent.h>
#include "agent_thread.h"
AGENT_NAMESPACE_BEGIN

class CAgentContext: public IAgent
{
public:
    CAgentContext(const TAgentInfo& agent_info);
    ~CAgentContext();

    void create();
    void destroy();

    const TAgentInfo& get_agent_info() const
    {
        return _agent_info;
    }
    
private: // override
    virtual void set_center(const std::string& domainname_or_iplist, uint16_t port);    
    virtual bool report(const char* data, size_t data_size, uint32_t timeout_millisecond=0);
    virtual bool report(uint32_t timeout_millisecond, const char* format, ...);
    virtual bool register_command_processor(ICommandProcessor* processor);
    virtual void deregister_command_processor(ICommandProcessor* processor);
    
private:
    TAgentInfo _agent_info;
    CAgentThread* _agent_thread;
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_CONTEXT_H

