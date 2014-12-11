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
#include <net/util.h>
#include "config_impl.h"

bool CConfigImpl::init(uint16_t port)
{
    try
    {        
        net::eth_ip_array_t eth_ip_array;
        net::CUtil::get_ethx_ip(eth_ip_array);

        // 设置默认的监听端口
        if (0 == port) 
        {
            port = 2012;
        }

        // 取得网卡上所有的IP地址
        for (int i=0; i<(int)eth_ip_array.size(); ++i)
        {
            net::ip_port_pair_t ip_port_pair;
            ip_port_pair.first = eth_ip_array[i].second.c_str();
            ip_port_pair.second = port;

            _ip_port_pair_array.push_back(ip_port_pair);            
        }
        
        return !_ip_port_pair_array.empty();
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "Get IP error: %s.\n", ex.to_string().c_str());
        return false;
    }
}

const net::ip_port_pair_array_t& CConfigImpl::get_listen_parameter() const
{
    return _ip_port_pair_array;
}
