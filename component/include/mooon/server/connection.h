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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef MOOON_SERVER_CONNECTION_H
#define MOOON_SERVER_CONNECTION_H
#include <mooon/net/ip_address.h>
#include <mooon/sys/log.h>
#include <mooon/server/config.h>
SERVER_NAMESPACE_BEGIN

/***
  * 网络连接
  */
class IConnection
{
public:        
    virtual ~IConnection() {}
    
    /** 得到字符串格式的标识 */
    virtual std::string str() const = 0;

    /** 得到本端的端口号 */
    virtual net::port_t self_port() const = 0;
    
    /** 得到对端的端口号 */
    virtual net::port_t peer_port() const = 0;
    
    /** 得到本端的IP地址 */
    virtual const net::ip_address_t& self_ip() const = 0;
    
    /** 得到对端的IP地址 */
    virtual const net::ip_address_t& peer_ip() const = 0;

    /** 得到所在线程的顺序号 */
    virtual uint16_t get_thread_index() const = 0;
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_CONNECTION_H
