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
#ifndef MOOON_NET_EPOLLABLE_H
#define MOOON_NET_EPOLLABLE_H
#include <fcntl.h>
#include <net/if.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "mooon/net/ip_address.h"
#include "mooon/net/utils.h"
#include "mooon/sys/ref_countable.h"
#include "mooon/sys/syscall_exception.h"
NET_NAMESPACE_BEGIN

/***
  * handle_epoll_event的返回值类型
  */
typedef enum
{
    epoll_none            = 0,       /** 调用者什么也不用做 */
    epoll_read            = 1,       /** 需要Epoll设置为只读事件 */
    epoll_write           = 2,       /** 需要Epoll设置为只写事件 */
    epoll_read_write      = 3,       /** 需要Epoll设置为读和写事件 */
    epoll_close           = 4,       /** 需要从Epoll中剔除，并关闭 */
    epoll_remove          = 5,       /** 需要从Epoll中剔除，但不关闭 */
    epoll_destroy         = 6,       /** 需要从Epoll中剔除，并且对象应当被销毁 */
    epoll_release         = 7        /** 释放控制权 */
}epoll_event_t;

/***
  * 判断指定fd是否具有指定的标志
  * @fd: 文件或套接字等句柄
  * @flags: 指定的标志值
  * @return: 如果具有指定的标志值，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool has_the_flags(int fd, int flags) throw (sys::CSyscallException);

/***
  * 判断指定fd是否为非阻塞的
  * @fd: 文件或套接字等句柄
  * @return: 如果fd为非阻塞的，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool is_nonblock(int fd) throw (sys::CSyscallException);

/***
  * 判断指定fd是否为非延迟的
  * @fd: 文件或套接字等句柄
  * @return: 如果fd为非延迟的，则返回true，否则返回false
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
bool is_nodelay(int fd) throw (sys::CSyscallException);

/***
  * 为指定的fd增加或删除指定的标志
  * @fd: 文件或套接字等句柄
  * @yes: 是否设置为指定标志，如果为true，则增加status指定的标志，否则去掉status指定的标志
  * @flags: 标志值
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_socket_flags(int fd, bool yes, int flags) throw (sys::CSyscallException);

void set_linger(int fd, bool onoff, int linger_interval) throw (sys::CSyscallException);

/***
  * 设置TCP选项（TCP_CORK，TCP_NODELAY）
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_tcp_option(int fd, bool yes, int option) throw (sys::CSyscallException);

/***
  * 为指定fd的增加或删除非阻塞标志
  * @fd: 文件或套接字等句柄
  * @yes: 是否设置为非阻塞标志，如果为true，则设置为非阻塞，否则设置为阻塞
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_nonblock(int fd, bool yes) throw (sys::CSyscallException);

/***
  * 为指定fd的增加或删除非延迟标志
  * @fd: 文件或套接字等句柄
  * @yes: 是否设置为非延迟标志，如果为true，则设置为非延迟，否则设置为延迟
  * @exception: 如果发生错误，则抛出CSyscallException异常
  */
void set_nodelay(int fd, bool yes) throw (sys::CSyscallException);

/** 关闭指定的句柄
  */
void close_fd(int fd) throw ();

/***
  * 得到已经发送的文件总字节数
  */
long get_send_file_bytes();

/***
  * 得到已经发送的不包括文件在内的总字节数
  */
long get_send_buffer_bytes();

/***
  * 得到已经接收的数据总字节数
  */
long get_recv_buffer_bytes();

//////////////////////////////////////////////////////////////////////////
// CEpollable

/***
  * 可Epool类，所有可使用Epoll监控对象的基类
  */
class CEpollable: public sys::CRefCountable
{
    friend class CEpoller;

public:
    CEpollable();
    virtual ~CEpollable();

    /** 关闭句柄 */
    virtual void close();

    /** 关闭读通道 */
    virtual void close_read();

    /** 关闭写通道 */
    virtual void close_write();

    /** 关闭读和写通道 */
    virtual void close_both();

    /** 解除对文件句柄的关联 */
    void detach() { _fd = -1; }
    
    /** 得到句柄值 */
    int get_fd() const { return _fd; }
    
    /** 得到设置的Epoll事件 */
    int get_epoll_events() const { return _epoll_events; }

    /***
      * 判断指定fd是否为非阻塞的
      * @return: 如果fd为非阻塞的，则返回true，否则返回false
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    bool is_nonblock();

    /***
      * 判断指定fd是否为非延迟的
      * @return: 如果fd为非延迟的，则返回true，否则返回false
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    bool is_nodelay();

    /***
      * 修改对象的非阻塞属性
      * @yes: 是否设置为非阻塞标识，如果为true，则设置为非阻塞，否则设置为阻塞
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
	void set_nonblock(bool yes);

    /***
      * 修改对象的非延迟属性
      * @yes: 是否设置为非延迟标识，如果为true，则设置为非延迟，否则设置为延迟
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
	void set_nodelay(bool yes);

    /** 得到socket错误代码 */
    int get_socket_error_code();

    /** 得到socket错误信息 */
    std::string get_socket_error_message();

public:
    /***
      * Epoll事件回调函数
      * @ptr: 对象指针
      * @events: 发生的Epoll事件
      * @return: 请参见epoll_event_t的说明
      * @exception: 系统调用出错，抛出CSyscallException异常
      */
    virtual epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
    {
        return net::epoll_close;
    }

protected: // 供继承的子类使用
    /***
      * 注意它不会调用before_close
      */
    void do_close();
    
    /** 设置句柄(句柄也称作文件描述符) */
    void set_fd(int fd) { _fd = fd; }    

    /** close之前被调用 */
    virtual void before_close();

private:    
    void set_epoll_events(int epoll_events)
    {
        _epoll_events = epoll_events;
    }

private:
    int _fd;
    int _epoll_events;
};

NET_NAMESPACE_END
#endif // MOOON_NET_EPOLLABLE_H
