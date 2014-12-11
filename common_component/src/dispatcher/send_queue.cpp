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
#include "sender.h"
#include "send_queue.h"
#include "send_thread.h"
DISPATCHER_NAMESPACE_BEGIN

CSendQueue::CSendQueue(uint32_t queue_max, CSender* sender)
    :net::CEpollableQueue<util::CArrayQueue<message_t*> >(queue_max)
    ,_sender(sender)
{
}

net::epoll_event_t CSendQueue::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{
    // 通知CSender去发送消息
    CSendThread* thread = static_cast<CSendThread*>(input_ptr);
    net::CEpoller& epoller = thread->get_epoller();

    epoller.set_events(_sender, EPOLLIN|EPOLLOUT);
    return net::epoll_remove;
}

DISPATCHER_NAMESPACE_END
