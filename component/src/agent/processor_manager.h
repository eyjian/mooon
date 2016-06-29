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
#ifndef MOOON_AGENT_PROCESSOR_MANAGER_H
#define MOOON_AGENT_PROCESSOR_MANAGER_H
#include <map>
#include <mooon/agent/command_processor.h>
#include <mooon/agent/message.h>
#include <mooon/sys/lock.h>
AGENT_NAMESPACE_BEGIN
    
class CProcessorManager
{
public:
    /***
      * register_processor & deregister_processor called by CAgentContext
      */
    bool register_processor(ICommandProcessor* processor);
    void deregister_processor(ICommandProcessor* processor);
    
    /***
      * called by CRecvMachine
      */
    bool on_header(const net::TCommonMessageHeader& header);

    /***
      * called by CRecvMachine
      */
    bool on_message(const net::TCommonMessageHeader& header
                  , size_t finished_size
                  , const char* buffer
                  , size_t buffer_size);

private:
    ICommandProcessor* get_command_processor(uint32_t command);

private:
    sys::CLock _lock; // used to protect _processor_map
    typedef std::map<uint32_t, ICommandProcessor*> CommandProcessorMap; // key is command code
    CommandProcessorMap _processor_map;
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_PROCESSOR_MANAGER_H
