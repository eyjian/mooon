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
#include <signal.h>
#include "net/utils.h"
#include "net/epollable.h"
NET_NAMESPACE_BEGIN

// CIgnorePipeSignal的作用是用来自动将PIPE信号忽略
static class CIgnorePipeSignal
{
public:
    CIgnorePipeSignal()
    {
        signal(SIGPIPE, SIG_IGN);
    }
}_do_not_used_for_ever; // 永远不要使用_do_not_used_for_ever

//////////////////////////////////////////////////////////////////////////
// 全局函数

/***
  * 判断指定fd是否具有指定的标志
  * @fd: 文件或套接字等句柄
  * @flags: 指定的标志值
  * @return: 如果具有指定的标志值，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool has_the_flags(int fd, int flags) throw (sys::CSyscallException)
{
    int curr_flags = fcntl(fd, F_GETFL, 0);
    if (-1 == curr_flags)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fcntl");

    return (curr_flags & flags) == flags;
}

/***
  * 判断指定fd是否为非阻塞的
  * @fd: 文件或套接字等句柄
  * @return: 如果fd为非阻塞的，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool is_nonblock(int fd) throw (sys::CSyscallException)
{
    return has_the_flags(fd, O_NONBLOCK);
}

/***
  * 判断指定fd是否为非延迟的
  * @fd: 文件或套接字等句柄
  * @return: 如果fd为非延迟的，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool is_nodelay(int fd) throw (sys::CSyscallException)
{
    return has_the_flags(fd, O_NDELAY);
}

/***
  * 为指定的fd增加或删除指定的标志
  * @fd: 文件或套接字等句柄
  * @yes: 是否设置为指定标志，如果为true，则增加status指定的标志，否则去掉status指定的标志
  * @flags: 标志值
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_socket_flags(int fd, bool yes, int flags) throw (sys::CSyscallException)
{
	// Get the file status flags
	int curr_flags = fcntl(fd, F_GETFL, 0);
	if (-1 == curr_flags)
	    THROW_SYSCALL_EXCEPTION(NULL, errno, "fcntl");

	// keep and set the file status flags
	int new_flags = yes? (curr_flags | flags): (curr_flags & ~flags);
	if (-1 == fcntl(fd, F_SETFL, new_flags))
	    THROW_SYSCALL_EXCEPTION(NULL, errno, "fcntl");
}

void set_linger(int fd, bool onoff, int linger_interval) throw (sys::CSyscallException)
{
    struct linger linger;
    linger.l_onoff = onoff? 1: 0;
    linger.l_linger = linger_interval;

    if (-1 == setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "setsockopt");
}

void set_tcp_option(int fd, bool yes, int option) throw (sys::CSyscallException)
{
    // TCP_CORK
    int on = yes? 1: 0;
    if (-1 == setsockopt(fd, SOL_TCP, option, &on, sizeof(on)))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "setsockopt");
}

/***
  * 为指定fd的增加或删除非阻塞标志
  * @fd: 文件或套接字等句柄
  * @yes: 是否设置为非阻塞标志，如果为true，则设置为非阻塞，否则设置为阻塞
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_nonblock(int fd, bool yes) throw (sys::CSyscallException)
{
    set_socket_flags(fd, yes, O_NONBLOCK);
}

/***
  * 为指定fd的增加或删除非延迟标志
  * @fd: 文件或套接字等句柄
  * @yes: 是否设置为非延迟标志，如果为true，则设置为非延迟，否则设置为延迟
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_nodelay(int fd, bool yes) throw (sys::CSyscallException)
{
    set_socket_flags(fd, yes, O_NDELAY);
}

void close_fd(int fd) throw ()
{
    (void)::close(fd);
}

//////////////////////////////////////////////////////////////////////////
// CEpollable

CEpollable::CEpollable()
    :_fd(-1)
    ,_epoll_events(-1)
{
}

CEpollable::~CEpollable()
{
    this->close();
}

void CEpollable::do_close()
{       
    if (_fd != -1)
    {        
        _epoll_events = -1;
        close_fd(_fd);
        _fd = -1; 
    }
}

void CEpollable::close()
{
    if (_fd != -1)
    {
        before_close();
        do_close();
    }    
}

void CEpollable::close_read()
{
    if (_fd != -1)
        shutdown(_fd, SHUT_RD);
}

void CEpollable::close_write()
{
    if (_fd != -1)
        shutdown(_fd, SHUT_WR);
}

void CEpollable::close_both()
{
    if (_fd != -1)
        shutdown(_fd, SHUT_RDWR);
}

/***
  * 判断指定fd是否为非阻塞的
  * @return: 如果fd为非阻塞的，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool CEpollable::is_nonblock()
{
    return net::is_nonblock(_fd);
}

/***
  * 判断指定fd是否为非延迟的
  * @return: 如果fd为非延迟的，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool CEpollable::is_nodelay()
{
    return net::is_nodelay(_fd);
}

/***
  * 修改对象的非阻塞属性
  * @yes: 是否设置为非阻塞标识，如果为true，则设置为非阻塞，否则设置为阻塞
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void CEpollable::set_nonblock(bool yes)
{
	net::set_nonblock(_fd, yes);
}

/***
  * 修改对象的非延迟属性
  * @yes: 是否设置为非延迟标识，如果为true，则设置为非延迟，否则设置为延迟
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void CEpollable::set_nodelay(bool yes)
{
	net::set_nodelay(_fd, yes);
}

int CEpollable::get_socket_error_code()
{
    int error_code;
	socklen_t error_code_length = sizeof(int);
	
	if (-1 == getsockopt(_fd, SOL_SOCKET, SO_ERROR, &error_code, &error_code_length))
    {
        return errno;
    }
    else
    {
        return error_code;
    }
}

std::string CEpollable::get_socket_error_message()
{
    return strerror(get_socket_error_code());
}

void CEpollable::before_close()
{
}

NET_NAMESPACE_END
