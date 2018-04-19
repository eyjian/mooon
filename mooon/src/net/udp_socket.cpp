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
#include "mooon/net/udp_socket.h"
#include "mooon/net/utils.h"
#include <unistd.h>
NET_NAMESPACE_BEGIN

CUdpSocket::CUdpSocket() throw (sys::CSyscallException)
{
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "socket");

    set_fd(fd);
}

void CUdpSocket::listen(uint16_t port, bool nonblock, bool reuse_port) throw (sys::CSyscallException)
{
	listen("0.0.0.0", port, nonblock, reuse_port);
}

void CUdpSocket::listen(const std::string& ip, uint16_t port, bool nonblock, bool reuse_port) throw (sys::CSyscallException)
{
	struct sockaddr_in listen_addr;

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(port);
	listen_addr.sin_addr.s_addr = inet_addr(ip.c_str()); // htonl(INADDR_ANY)
	memset(listen_addr.sin_zero, 0, sizeof(listen_addr.sin_zero));

    // IP地址重用
    int reuse = 1;
    int retval = ::setsockopt(get_fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (-1 == retval)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "setsockopt");

#if defined(SO_REUSEPORT)
    // 重用端口
    if (reuse_port)
    {
        // #define SO_REUSEPORT 15
        reuse = 1;
        retval = ::setsockopt(get_fd(), SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
        if (-1 == retval)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "setsockopt");
    }
#endif // SO_REUSEPORT

	if (-1 == bind(get_fd(), (struct sockaddr*)&listen_addr, sizeof(listen_addr)))
		THROW_SYSCALL_EXCEPTION(NULL, errno, "bind");

	if (nonblock)
	    set_nonblock(true);
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
    {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "sendto");
    }

    return bytes;
}

int CUdpSocket::receive_from(void* buffer, size_t buffer_size, uint32_t* from_ip, uint16_t* from_port) throw (sys::CSyscallException)
{
    struct sockaddr_in from_addr;

    int bytes = receive_from(buffer, buffer_size, &from_addr);
    if (bytes != -1)
    {
        *from_ip = from_addr.sin_addr.s_addr;
        *from_port = ntohs(from_addr.sin_port);
    }

    return bytes;
}

int CUdpSocket::receive_from(void* buffer, size_t buffer_size, struct sockaddr_in* from_addr) throw (sys::CSyscallException)
{
    socklen_t address_len = sizeof(struct sockaddr_in);

    int bytes = recvfrom(get_fd(), buffer, buffer_size, 0, (struct sockaddr*)from_addr, &address_len);
    if (-1 == bytes)
    {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "recvfrom");
    }

    return bytes;
}

// UDP无应用层发送缓存区，数据直接进入网卡输出队列，如果网卡输出队列已满，则设置errno值为ENOBUFS，
// 但根据man 2 sendto说明，一般情况下，Linux不会出现ENOBUFS，一旦网卡输出队列满则直接丢弃数据。
int CUdpSocket::timed_receive_from(void* buffer, size_t buffer_size, uint32_t* from_ip, uint16_t* from_port, uint32_t milliseconds) throw (sys::CSyscallException)
{
    if (!CUtils::timed_poll(get_fd(), POLLIN, milliseconds))
        THROW_SYSCALL_EXCEPTION("receive timeout", ETIMEDOUT, "pool");

    return receive_from(buffer, buffer_size, from_ip, from_port);
}

int CUdpSocket::timed_receive_from(void* buffer, size_t buffer_size, struct sockaddr_in* from_addr, uint32_t milliseconds) throw (sys::CSyscallException)
{
    if (!CUtils::timed_poll(get_fd(), POLLIN, milliseconds))
        THROW_SYSCALL_EXCEPTION("receive timeout", ETIMEDOUT, "pool");

    return receive_from(buffer, buffer_size, from_addr);
}

NET_NAMESPACE_END
