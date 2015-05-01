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
 *
 * 20110206: 将所有complete改为full
 */
#ifndef MOOON_NET_TCP_WAITER_H
#define MOOON_NET_TCP_WAITER_H
#include "mooon/net/epollable.h"
#include "mooon/net/ip_address.h"
#include <sys/uio.h>
NET_NAMESPACE_BEGIN

/***
  * TCP服务端类，提供服务端的各种功能
  */
class CTcpWaiter: public CEpollable
{
public:
	CTcpWaiter();
	~CTcpWaiter();

    /** 得到字符串格式的身份 */
    virtual std::string to_string() const;

    /** 得到对端的IP地址，调用attach后者才可用 */
    const ip_address_t& get_peer_ip() const { return _peer_ip; }

    /** 得到本端的地址 */
    const ip_address_t& get_self_ip() const { return _self_ip; }

    /** 得到对端的端口号，调用attach后者才可用 */
    port_t get_peer_port() const { return _peer_port; }

    /** 得到本端的端口号 */
    port_t get_self_port() const { return _self_port; }

    /** 设置本端的地址和端口 */
    void set_self(const ip_address_t& self_ip, port_t self_port);

    /***
      * 关联到一个fd
      * @fd: 被关闭的fd，fd为CListener::accept的返回值
      * @peer_ip: 对端的IP地址
      * @peer_port: 对端的端口号
      */
    void attach(int fd, const ip_address_t& peer_ip, port_t peer_port);

    /** 接收SOCKET数据
      * @buffer: 接收缓冲区
      * @buffer_size: 接收缓冲区字节数
      * @return: 如果收到数据，则返回收到的字节数；如果对端关闭了连接，则返回0；
      *          对于非阻塞连接，如果无数据可接收，则返回-1
      * @exception: 连接错误，抛出CSyscallException异常
      */
    ssize_t receive(char* buffer, size_t buffer_size);

    /** 发送SOCKET数据
      * @buffer: 发送缓冲区
      * @buffer_size: 需要发送的字节大小
      * @return: 如果发送成功，则返回实际发送的字节数；对于非阻塞的连接，如果不能继续发送，则返回-1
      * @exception: 如果发生网络错误，则抛出CSyscallException异常
      * @注意事项：保证不发送0字节的数据，也就是buffer_size必须大于0
      */
    ssize_t send(const char* buffer, size_t buffer_size);

    /***
      * 以超时方式接收数据，如果在指定的时间内未接收完，则返回
      * @timeout_milliseconds: 超时毫秒数
      * @return 返回实际已经接收的字节数，如果小于buffer_size，则表示接收超时
      * @exception: 如果发生网络错误，则抛出CSyscallException异常
      */
    ssize_t timed_receive(char* buffer, size_t buffer_size, uint32_t milliseconds);

    /***
      * 以超时方式发送数据，如果在指定的时间内未发送完，则返回
      * @milliseconds: 超时毫秒数
      * @return 返回实际已经发送的字节数，如果小于buffer_size，则表示发送超时
      * @exception: 如果发生网络错误，则抛出CSyscallException异常
      */
    ssize_t timed_send(const char* buffer, size_t buffer_size, uint32_t milliseconds);

    /** 完整接收，如果成功返回，则一定接收了指定字节数的数据
      * @buffer: 接收缓冲区
      * @buffer_size: 接收缓冲区字节大小，返回实际已经接收到的字节数(不管成功还是失败或异常)
      * @return: 如果成功，则返回true，否则如果连接被对端关闭则返回false
      * @exception: 如果发生网络错误，则抛出CSyscallException，对于非阻塞连接，也可能抛出CSyscallException异常
      */
    bool full_receive(char* buffer, size_t& buffer_size);

    /** 完整发送，如果成功返回，则总是发送了指定字节数的数据
      * @buffer: 发送缓冲区
      * @buffer_size: 需要发送的字节数，返回实际已经接发送了的字节数(不管成功还是失败或异常)
      * @return: 无返回值
      * @exception: 如果网络错误，则抛出CSyscallException异常；对于非阻塞连接，也可能抛出CSyscallException异常
      * @注意事项：保证不发送0字节的数据，也就是buffer_size必须大于0
      */
    void full_send(const char* buffer, size_t& buffer_size);

    /** 发送文件，调用者必须保证offset+count不超过文件大小
      * @file_fd: 打开的文件句柄
      * @offset: 文件偏移位置，如果成功则返回新的偏移位置
      * @count: 需要发送的大小
      */
    ssize_t send_file(int file_fd, off_t *offset, size_t count);
    void full_send_file(int file_fd, off_t *offset, size_t& count);

    /** 采用内存映射的方式接收，并将数据存放文件，适合文件不是太大
      * @file_fd: 打开的文件句柄
      * @size: 需要写入文件的大小，返回实际已经接收到的字节数(不管成功还是失败或异常)
      * @offset: 写入文件的偏移值
      * @return: 如果连接被对端关闭，则返回false否则成功返回true
      * @exception: 如果发生系统调用错误，则抛出CSyscallException异常
      */
    bool full_map_tofile(int file_fd, size_t& size, size_t offset);

    /** 采用write调用的方式接收，并将数据存放文件，适合任意大小的文件，但是大文件会导致该调用长时间阻塞
      * @file_fd: 打开的文件句柄
      * @size: 需要写入文件的大小，返回实际已经接收到的字节数(不管成功还是失败或异常)
      * @offset: 写入文件的偏移值
      * @return: 如果连接被对端关闭，则返回false否则成功返回true
      * @exception: 如果发生系统调用错误，则抛出CSyscallException异常
      */
    bool full_write_tofile(int file_fd, size_t& size, size_t offset);

    /***
      * 一次性读一组数据，和系统调用readv的用法相同
      * @return: 返回实际读取到的字节数
      * @exception: 如果发生系统调用错误，则抛出CSyscallException异常
      */
    ssize_t readv(const struct iovec *iov, int iovcnt);

    /***
      * 一次性写一组数据，和系统调用writev的用法相同
      * @return: 返回实际写入的字节数
      * @exception: 如果发生系统调用错误，则抛出CSyscallException异常
      */
    ssize_t writev(const struct iovec *iov, int iovcnt);

protected:
    std::string do_to_string() const;

private:
    void* _data_channel;    
    ip_address_t _self_ip; /** 本端的IP地址 */
    ip_address_t _peer_ip; /** 对端的IP地址 */
    port_t _self_port;     /** 本端的端口号 */
    port_t _peer_port;     /** 对端的端口号 */
};

NET_NAMESPACE_END
#endif // MOOON_NET_TCP_WAITER_H
