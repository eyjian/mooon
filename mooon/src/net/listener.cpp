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
#include <fcntl.h>
#include <sys/utils.h>
#include "net/utils.h"
#include "net/listener.h"
NET_NAMESPACE_BEGIN

CListener::CListener()
    :_port(0)
{
}

void CListener::listen(const ip_address_t& ip, uint16_t port, bool nonblock, bool enabled_address_zero, bool reuse_port) throw (sys::CSyscallException)
{    
    // 是否允许是任意地址上监听
    if (!enabled_address_zero && ip.is_zero_address())
        THROW_EXCEPTION("forbid listening on 0.0.0.0 address", EINVAL);

    // 禁止监听广播地址
    if ( ip.is_broadcast_address())
        THROW_EXCEPTION("forbid listening on broadcast address", EINVAL);

    socklen_t addr_len;           // 地址长度，如果为IPV6则等于sizeof(struct sockaddr_in6)，否则等于sizeof(struct sockaddr_in)
    struct sockaddr* addr;        // 监听地址，如果为IPV6则指向addr_in6，否则指向addr_in
    struct sockaddr_in addr_in;
    struct sockaddr_in6 addr_in6;
    
    const uint32_t* ip_data = ip.get_address_data();
    if (ip.is_ipv6())
    {
        addr_len = sizeof(struct sockaddr_in6);
        addr = (struct sockaddr*)&addr_in6;
        addr_in6.sin6_family = AF_INET6;
        addr_in6.sin6_port = htons(port);
        memcpy(&addr_in6.sin6_addr, ip_data, addr_len);
    }
    else
    {
        addr_len = sizeof(struct sockaddr_in);
        addr = (struct sockaddr*)&addr_in;
        addr_in.sin_family = AF_INET;        
        addr_in.sin_port = htons(port);
        addr_in.sin_addr.s_addr = ip_data[0];
        memset(addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));
    }

    // 创建一个socket
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "socket");

    try
    {
        int retval; // 用来保存返回值

        // IP地址重用
        int reuse = 1;
        retval = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        if (-1 == retval)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "setsockopt");

#if defined(SO_REUSEPORT)
        // 重用端口
        if (reuse_port)
        {
            // #define SO_REUSEPORT 15
            reuse = 1;
            retval = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
            if (-1 == retval)
                THROW_SYSCALL_EXCEPTION(NULL, errno, "setsockopt");
        }
#endif // SO_REUSEPORT

        // 防止子进程继承
        retval = ::fcntl(fd, F_SETFD, FD_CLOEXEC);
        if (-1 == retval)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "fcntl");

        int retry_times = 50; // 重试次数
        for (;;)
        {
            retval = ::bind(fd, addr, addr_len);
            if (retval != -1) break;
            
            if ((EADDRINUSE == errno) && (--retry_times > 0))
                sys::CUtils::millisleep(100);
            else
                THROW_SYSCALL_EXCEPTION(NULL, errno, "bind");
        }

        // 如果没有bind，则会随机选一个IP和端口，所以listen之前必须有bind
        retval = ::listen(fd, 10000);
        if (-1 == retval)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "listen");

        // 设置为非阻塞模式
        if (nonblock)
            net::set_nonblock(fd, true);
        
        // 存储ip和port不是必须的，但可以方便gdb时查看对象的值
        _ip = ip;
        _port = port;
        CEpollable::set_fd(fd);
    }
    catch (...)
    {
        if (fd != -1) ::close(fd);
        throw;
    }
}

void CListener::listen(const ipv4_node_t& ip_node, bool nonblock, bool enabled_address_zero, bool reuse_port) throw (sys::CSyscallException)
{
    ip_address_t ip = ip_node.ip;
    listen(ip, ip_node.port, nonblock, enabled_address_zero);
}

void CListener::listen(const ipv6_node_t& ip_node, bool nonblock, bool enabled_address_zero, bool reuse_port) throw (sys::CSyscallException)
{
    ip_address_t ip = (uint32_t*)ip_node.ip;
    listen(ip, ip_node.port, nonblock, enabled_address_zero);
}

int CListener::accept(ip_address_t& peer_ip, uint16_t& peer_port) throw (sys::CSyscallException)
{
    struct sockaddr_in6 peer_addr_in6;
    struct sockaddr* peer_addr = (struct sockaddr*)&peer_addr_in6;        
    socklen_t peer_addrlen = sizeof(struct sockaddr_in6); // 使用最大的

    int newfd = ::accept(CEpollable::get_fd(), peer_addr, &peer_addrlen);
    if (-1 == newfd) 
    {
        if (sys::Error::code() != EWOULDBLOCK)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "accept");
        
        return -1;      
    }

    // 接受的是一个IPV4请求
    if (AF_INET == peer_addr->sa_family)
    {
        struct sockaddr_in* peer_addr_in = (struct sockaddr_in*)peer_addr;
        peer_port = peer_addr_in->sin_port;
        peer_ip = peer_addr_in->sin_addr.s_addr;
    }
    else
    {
        // 接受的是一个IPV6请求
        peer_port = peer_addr_in6.sin6_port;
        peer_ip = (uint32_t*)&peer_addr_in6.sin6_addr;
    }

    return newfd;
}

NET_NAMESPACE_END
