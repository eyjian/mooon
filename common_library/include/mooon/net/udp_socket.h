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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_NET_UDP_SOCKET_H
#define MOOON_NET_UDP_SOCKET_H
#include "mooon/net/epollable.h"
NET_NAMESPACE_BEGIN

// UDP不分服务端和客户端，
// 但如果仅做服务端或即做服务端又做客户端时，都必须调用listen()，
// 仅做客户端使用时，可不必调用listen()
// 多数设备以太网链路层MTU（最大传输单元）大小为1500（但标准为576），IP包头为20，TCP包头为20，UDP包头8
class CUdpSocket: public CEpollable
{
public:
    CUdpSocket() throw (sys::CSyscallException);
    void listen(uint16_t port, bool nonblock=false) throw (sys::CSyscallException);
    void listen(const std::string& ip, uint16_t port, bool nonblock=false) throw (sys::CSyscallException);

    // to_ip 目标IP，要求为网络字节序
    // to_port 目标端口，同样要求为主机字节序
    int send_to(const void* buffer, size_t buffer_size, uint32_t to_ip, uint16_t to_port) throw (sys::CSyscallException);
    int send_to(const void* buffer, size_t buffer_size, const char* to_ip, uint16_t to_port) throw (sys::CSyscallException);
    int send_to(const void* buffer, size_t buffer_size, const struct sockaddr_in& to_addr) throw (sys::CSyscallException);

    // from_ip 返回的源IP，为网络字节序
    // from_port 返回的源端口，为主机字节序
    // 出错抛出异常，成功返回接收的字节数，如果返回-1表示为非阻塞模式没有数据可接收
    int receive_from(void* buffer, size_t buffer_size, uint32_t* from_ip, uint16_t* from_port) throw (sys::CSyscallException);
    int receive_from(void* buffer, size_t buffer_size, struct sockaddr_in* from_addr) throw (sys::CSyscallException);

    int timed_receive_from(void* buffer, size_t buffer_size, uint32_t* from_ip, uint16_t* from_port, uint32_t milliseconds) throw (sys::CSyscallException);
    int timed_receive_from(void* buffer, size_t buffer_size, struct sockaddr_in* from_addr, uint32_t milliseconds) throw (sys::CSyscallException);
};

NET_NAMESPACE_END
#endif // MOOON_NET_UDP_SOCKET_H
