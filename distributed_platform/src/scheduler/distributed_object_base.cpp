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
#include "distributed_object_base.h"
SCHED_NAMESPACE_BEGIN

CDistributedObjectBase::CDistributedObjectBase()
{
    init_message_proc();
}

bool CDistributedObjectBase::on_message(const TDistributedMessage* message)
{
    int index = command2index(message->header.command);
    return (-1 == index)
          ? on_user_message(message)
          : (*_message_proc[index])(message);
}

void CDistributedObjectBase::init_message_proc()
{
    _message_proc[REQUEST_COMMAND]         = &do_request;
    _message_proc[RESPONSE_COMMAND]        = &do_response;
    _message_proc[TIMER_COMMAND]           = &do_timer;
    _message_proc[CREATE_SESSION_COMMAND]  = &do_create_session;
    _message_proc[DESTROY_SESSION_COMMAND] = &do_destroy_session;
}

bool CDistributedObjectBase::do_request(const TDistributedMessage* message)
{
    // callback
    return on_request(message);
}

bool CDistributedObjectBase::do_response(const TDistributedMessage* message)
{
    // callback
    return on_response(message);
}

bool CDistributedObjectBase::do_timer(const TDistributedMessage* message)
{
    // callback
    return on_timer(message);
}

bool CDistributedObjectBase::do_create_session(const TDistributedMessage* message)
{
    // callback
    return do_create_session(message);
}

bool CDistributedObjectBase::do_destroy_session(const TDistributedMessage* message)
{
    // callback
    return on_destroy_session(message);
}

SCHED_NAMESPACE_END
