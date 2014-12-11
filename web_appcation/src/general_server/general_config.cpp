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
#include "general_config.h"
MOOON_NAMESPACE_BEGIN

CGeneralConfig::CGeneralConfig()
{
    _ip_port_pair = new net::ip_port_pair_t;
    _ip_port_pair->first = "127.0.0.1";
    _ip_port_pair->second = 6789;

    _ip_port_pair_array.push_back(_ip_port_pair);
}

CGeneralConfig::~CGeneralConfig()
{
    delete _ip_port_pair;
}

/** 得到epoll大小 */
uint32_t CGeneralConfig::get_epoll_size() const
{
    return 10000;
}
    
/** 得到epool等待超时毫秒数 */
uint32_t CGeneralConfig::get_epoll_timeout() const
{
    return 1000;
}

/** 得到框架的工作线程个数 */
uint16_t CGeneralConfig::get_thread_number() const
{
    return 1;
}

/** 得到连接池大小 */
uint32_t CGeneralConfig::get_connection_pool_size() const
{
    return 10000;
}

/** 连接超时秒数 */
uint32_t CGeneralConfig::get_connection_timeout_seconds() const
{
    return 60;
}

/** 得到监听参数 */    
const net::ip_port_pair_array_t& CGeneralConfig::get_listen_parameter() const
{    
    return _ip_port_pair_array;
}

MOOON_NAMESPACE_END
