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
#include "net/utils.h"
#include "net/ip_address.h"
NET_NAMESPACE_BEGIN

std::string ip_address_t::to_string() const
{
    return _is_ipv6? net::CUtils::ipv6_tostring(_ip_data): net::CUtils::ipv4_tostring(_ip_data[0]);
}

uint32_t ip_address_t::to_ipv4() const
{
    return _ip_data[0];
}

const uint32_t* ip_address_t::to_ipv6() const
{
    return _ip_data;
}

size_t ip_address_t::get_address_data_length() const
{
    return _is_ipv6? 16: 4;
}

const uint32_t* ip_address_t::get_address_data() const
{
    return _ip_data;
}

bool ip_address_t::is_zero_address() const
{
    return !_is_ipv6 && (0 == _ip_data[0]);
}

bool ip_address_t::is_broadcast_address() const
{
    return CUtils::is_broadcast_address(to_string().c_str());
}

void ip_address_t::from_string(const char* ip) throw (utils::CException)
{
    if (NULL == ip)
    {
        _is_ipv6 = false;
        _ip_data[0] = 0;
        _ip_data[1] = 0;
        _ip_data[2] = 0;
        _ip_data[3] = 0;
    }
    else if (net::CUtils::string_toipv4(ip, _ip_data[0]))
    {
        _is_ipv6 = false;
        _ip_data[1] = 0;
        _ip_data[2] = 0;
        _ip_data[3] = 0;
    }
    else if (net::CUtils::string_toipv6(ip, _ip_data))
    {
        _is_ipv6 = true;
    }
    else
    {
        THROW_EXCEPTION(ip, EINVAL);
    }
}

NET_NAMESPACE_END
