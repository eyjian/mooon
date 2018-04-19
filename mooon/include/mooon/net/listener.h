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
#ifndef MOOON_NET_LISTENER_H
#define MOOON_NET_LISTENER_H
#include "mooon/net/ip_node.h"
#include "mooon/net/epollable.h"
NET_NAMESPACE_BEGIN

/***
  * TCP服务端监听者类
  * 用于启动在某端口上的监听和接受连接请求
  */
class CListener: public CEpollable
{
public:
    /** 构造一个TCP监控者 */
    CListener();

    /** 是否为IPV6监听者 */
    bool is_ipv6() const throw () { return _ip.is_ipv6(); }
    
    /***
      * 启动在指定IP和端口上的监听
      * @ip: 监听IP地址
      * @port: 监听端口号
      * @enabled_address_zero: 是否允许在0.0.0.0上监听，安全起见，默认不允许
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    void listen(const ip_address_t& ip, uint16_t port, bool nonblock=true, bool enabled_address_zero=false, bool reuse_port=false) throw (sys::CSyscallException);
    void listen(const ipv4_node_t& ip_node, bool nonblock=true, bool enabled_address_zero=false, bool reuse_port=false) throw (sys::CSyscallException);
    void listen(const ipv6_node_t& ip_node, bool nonblock=true, bool enabled_address_zero=false, bool reuse_port=false) throw (sys::CSyscallException);

    /***
      * 接受连接请求
      * @peer_ip: 用来存储对端的IP地址
      * @peer_port: 用来存储对端端口号
      * @return: 新的SOCKET句柄
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    int accept(ip_address_t& peer_ip, uint16_t& peer_port) throw (sys::CSyscallException);
    
    /** 得到监听的IP地址 */
    const ip_address_t& get_listen_ip() const throw () { return _ip; }

    /** 得到监听的端口号 */
    uint16_t get_listen_port() const throw () { return _port; }

private:    
    uint16_t _port;
    ip_address_t _ip;
};

NET_NAMESPACE_END
#endif // MOOON_NET_LISTENER_H
