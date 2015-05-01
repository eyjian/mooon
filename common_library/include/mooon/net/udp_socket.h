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
#include "net/epollable.h"
NET_NAMESPACE_BEGIN

// UDP不分服务端和客户端，
// 但如果仅做服务端或即做服务端又做客户端时，都必须调用listen()，
// 仅做客户端使用时，可不必调用listen()
class CUdpSocket: public CEpollable
{
public:
    CUdpSocket() throw (sys::CSyscallException);
    void listen(uint16_t port) throw (sys::CSyscallException);

    int send_to(const void* buffer, size_t buffer_size, uint32_t to_ip, uint16_t to_port) throw (sys::CSyscallException);
    int send_to(const void* buffer, size_t buffer_size, const char* to_ip, uint16_t to_port) throw (sys::CSyscallException);
    int send_to(const void* buffer, size_t buffer_size, const struct sockaddr_in& to_addr) throw (sys::CSyscallException);

    int receive_from(void* buffer, size_t buffer_size, uint32_t* from_ip, uint16_t* from_port) throw (sys::CSyscallException);
    int receive_from(void* buffer, size_t buffer_size, struct sockaddr_in* from_addr) throw (sys::CSyscallException);
};

NET_NAMESPACE_END
#endif // MOOON_NET_UDP_SOCKET_H
