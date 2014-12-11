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

#include <net/util.h>
#include <sys/util.h>
#include "config_impl.h"
#include "pp_log.h"

PP_NAMESPACE_BEGIN

bool CConfigImpl::init(const std::string& ip, uint16_t port)
{
    net::ip_port_pair_t ip_port_pair;

    ip_port_pair.first = ip.c_str();
    ip_port_pair.second = port;
    _ip_port_pair_array.push_back(ip_port_pair);
}

const net::ip_port_pair_array_t& CConfigImpl::get_listen_parameter() const
{
    return _ip_port_pair_array;
}

uint16_t CConfigImpl::get_thread_number() const
{
    PP_LOG_INFO("[CConfigImpl::get_thread_number] %d\n", sys::CUtil::get_cpu_number());

    return (0 == sys::CUtil::get_cpu_number()*2) ? 1 : sys::CUtil::get_cpu_number()*2;
}

PP_NAMESPACE_END

