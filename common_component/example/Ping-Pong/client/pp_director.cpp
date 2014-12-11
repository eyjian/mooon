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

#include <net/inttypes.h>
#include "pp_director.h"
#include "pp_reply_handler.h"
#include "pp_msg.h"

PP_NAMESPACE_BEGIN

CppDirector::CppDirector()
 : _dispatcher(NULL)
{
    _sender_count = 1;
    _server_port = 0;
    _server_ip = 0;
}

void CppDirector::set_server_ip_port(uint32_t ip, uint16_t port)
{
    _server_ip = ip;
    _server_port = port;
}

void CppDirector::set_sender_count(int sender_count)
{
    _sender_count = sender_count;
}

void CppDirector::set_bytes_per_send(int bytes_per_send)
{
    _bytes_per_send = bytes_per_send;
}

void CppDirector::set_dispatcher(dispatcher::IDispatcher* dispatcher)
{
    _dispatcher = dispatcher;
}

bool CppDirector::start(void)
{
    dispatcher::ISender* sender;
    dispatcher::IManagedSenderTable* sender_table;

    MOOON_ASSERT(_dispatcher);
    MOOON_ASSERT(_server_port);
    MOOON_ASSERT(_server_ip);

    sender_table = _dispatcher->get_managed_sender_table();
    for (int i=0; i<_sender_count; ++i)
    {
        dispatcher::SenderInfo sender_info;
        dispatcher::buffer_message_t* buffer_message;

        fill_sender_info(sender_info, i);
        sender = sender_table->open_sender(sender_info);
        PP_LOG_INFO("Push message to %s.\n", sender->str().c_str());
	// create_pp_message 会使第一个参数 PING_PONG_MSG 的内存读越界，但没关系:)
        buffer_message = create_pp_message(PING_PONG_MSG, strlen(PING_PONG_MSG) + _bytes_per_send);
        //buffer_message = create_pp_message(PING_PONG_MSG, strlen(PING_PONG_MSG));
        sender->push_message(buffer_message);
        sender_table->release_sender(sender);
    }

    return true;
}

void CppDirector::fill_sender_info(dispatcher::SenderInfo& sender_info, uint16_t key)
{
    sender_info.key = key;
    sender_info.ip_node.port = _server_port;
    sender_info.ip_node.ip = _server_ip;        
    sender_info.queue_size = 1;
    sender_info.resend_times = -1;
    sender_info.reconnect_times = -1;
    sender_info.reply_handler = new CppReplyHandler();
}

PP_NAMESPACE_END

