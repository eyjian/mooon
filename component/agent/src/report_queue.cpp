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
#include "report_queue.h"
#include "agent_thread.h"
AGENT_NAMESPACE_BEGIN

CReportQueue::CReportQueue(uint32_t queue_max, CAgentThread* agent_thread)
 :net::CEpollableQueue<utils::CArrayQueue<net::TCommonMessageHeader*> >(queue_max)
 ,_agent_thread(agent_thread)
{
}

net::epoll_event_t CReportQueue::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{        
    _agent_thread->enable_connector_write();    
    return net::epoll_remove;
}

AGENT_NAMESPACE_END
