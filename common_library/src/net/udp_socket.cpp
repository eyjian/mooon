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
#include "net/udp_socket.h"
#include <unistd.h>
NET_NAMESPACE_BEGIN

CUdpSocket::CUdpSocket()
{
}

void CUdpSocket::listen(uint16_t port) throw (sys::CSyscallException)
{
    struct sockaddr_in listen_addr;

    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "socket");

    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(port);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(listen_addr.sin_zero, 0, sizeof(listen_addr.sin_zero));

    if (-1 == bind(fd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "bind");

    set_fd(fd);
}

int CUdpSocket::send_to(const void* buffer, size_t buffer_size, uint32_t to_ip, uint16_t to_port) throw (sys::CSyscallException)
{
    struct sockaddr_in to_addr;

    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(to_port);
    to_addr.sin_addr.s_addr = to_ip;
    memset(to_addr.sin_zero, 0, sizeof(to_addr.sin_zero));

    return send_to(buffer, buffer_size, to_addr);
}

int CUdpSocket::send_to(const void* buffer, size_t buffer_size, const char* to_ip, uint16_t to_port) throw (sys::CSyscallException)
{
    return send_to(buffer, buffer_size, inet_addr(to_ip), to_port);
}

int CUdpSocket::send_to(const void* buffer, size_t buffer_size, const struct sockaddr_in& to_addr) throw (sys::CSyscallException)
{
    int bytes = ::sendto(get_fd(), buffer, buffer_size, 0, (struct sockaddr*)&to_addr, sizeof(struct sockaddr_in));
    if (-1 == bytes)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "sendto");

    return bytes;
}

int CUdpSocket::receive_from(void* buffer, size_t buffer_size, uint32_t* from_ip, uint16_t* from_port) throw (sys::CSyscallException)
{
    struct sockaddr_in from_addr;

    int bytes = receive_from(buffer, buffer_size, &from_addr);
    *from_ip = from_addr.sin_addr.s_addr;
    *from_port = ntohs(from_addr.sin_port);

    return bytes;
}

int CUdpSocket::receive_from(void* buffer, size_t buffer_size, struct sockaddr_in* from_addr) throw (sys::CSyscallException)
{
    socklen_t address_len = sizeof(struct sockaddr_in);

    int bytes = recvfrom(get_fd(), buffer, buffer_size, 0, (struct sockaddr*)from_addr, &address_len);
    if (-1 == bytes)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "recvfrom");

    return bytes;
}

NET_NAMESPACE_END
