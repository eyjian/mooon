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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_SERVER_LISTENER_H
#define MOOON_SERVER_LISTENER_H
#include <mooon/sys/log.h>
#include <mooon/net/listener.h>
#include "log.h"
SERVER_NAMESPACE_BEGIN

class CListener: public net::CListener
{
private:
    virtual net::epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr);
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_LISTENER_H
