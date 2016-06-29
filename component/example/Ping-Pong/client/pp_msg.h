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
 * Author: laocai_liu@qq.com or laocailiu@gmail.com
 */

#ifndef MOOON_PP_MSG_H
#define MOOON_PP_MSG_H

#include "config.h"

PP_NAMESPACE_BEGIN

#define PING_PONG_MSG "ping pong message"

inline dispatcher::buffer_message_t* create_pp_message(const char *msg, int msg_len)
{
    int buf_length;
    int head_length;
    int payload_length;
    dispatcher::buffer_message_t* buffer_message;
    net::TCommonMessageHeader* msg_head;

    head_length = sizeof(net::TCommonMessageHeader);
    payload_length = msg_len;
    buf_length = head_length + payload_length;
    buffer_message = dispatcher::create_buffer_message(buf_length);
    msg_head = reinterpret_cast<net::TCommonMessageHeader *>(buffer_message->data);
    msg_head->size = payload_length;
    memcpy(buffer_message->data + head_length, msg, payload_length);

    return buffer_message;
}

PP_NAMESPACE_END

#endif
