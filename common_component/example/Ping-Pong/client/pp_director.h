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

#ifndef MOOON_PP_DIRECTOR_H
#define MOOON_PP_DIRECTOR_H

#include <dispatcher/dispatcher.h>
#include "pp_log.h"

PP_NAMESPACE_BEGIN

class CppDirector
{
public:
    CppDirector();

public:
    void set_server_ip_port(uint32_t ip, uint16_t port);
    void set_sender_count(int sender_count);
    void set_bytes_per_send(int bytes_per_send);
    void set_dispatcher(dispatcher::IDispatcher* dispatcher);
    bool start(void);

private:
    void fill_sender_info(dispatcher::SenderInfo& sender_info, uint16_t key);

private:
    uint32_t _server_ip;
    uint16_t _server_port;
    int _sender_count;
    int _bytes_per_send;
    dispatcher::IDispatcher* _dispatcher;
};

PP_NAMESPACE_END

#endif
