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
#ifndef MOOON_AGENT_QUEUE_H
#define MOOON_AGENT_QUEUE_H
#include <mooon/agent/message.h>
#include <mooon/net/epollable_queue.h>
#include <mooon/utils/array_queue.h>
AGENT_NAMESPACE_BEGIN

class CAgentThread;
class CReportQueue: public net::CEpollableQueue<util::CArrayQueue<net::TCommonMessageHeader*> >
{
public:
    CReportQueue(uint32_t queue_max, CAgentThread* agent_thread);
    
private:
    virtual net::epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr);
    
private:
    CAgentThread* _agent_thread;
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_QUEUE_H

