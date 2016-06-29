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

#ifndef MOOON_PP_CONFIG_IMPL_H
#define MOOON_PP_CONFIG_IMPL_H

#include <server/server.h>
#include "config.h"

PP_NAMESPACE_BEGIN

class CConfigImpl: public server::IConfig
{
public:
    bool init(const std::string& ip, uint16_t port);

private:
    virtual const net::ip_port_pair_array_t& get_listen_parameter() const;
    virtual uint16_t get_thread_number() const;

private:
    net::ip_port_pair_array_t _ip_port_pair_array;
};

PP_NAMESPACE_END

#endif // MOOON_PP_CONFIG_IMPL_H
