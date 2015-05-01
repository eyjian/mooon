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
 * Author: Jian Yi, eyjian@qq.com or eyjian@gmail.com
 *
 * 20110206: 将所有complete改为full
 */
#ifndef MOOON_NET_TCP_CLIENT_H
#define MOOON_NET_TCP_CLIENT_H
#include "mooon/net/ip_node.h"
#include "mooon/net/epollable.h"
NET_NAMESPACE_BEGIN

/***
  * TCP客户端类，提供客户端的各种功能
  */
class CTcpClient: public CEpollable
{
public:
	CTcpClient();
	~CTcpClient();

    /** 得到字符串格式的身份 */
    virtual std::string to_string() const;

    /** 是否为IPV6类型 */
    bool is_ipv6() const;

    /** 得到对端端口号 */
    uint16_t get_peer_port() const;

    /** 得到对端IP地址 */
    const ip_address_t& get_peer_ip() const;

    /** 设置对端的IP和端口号 */
    void set_peer(const ip_node_t& ip_node);
    void set_peer(const ipv4_node_t& ip_node);
    void set_peer(const ipv6_node_t& ip_node);

    /***
      * 设置对端IP地址
      * @ip: 对端的IP地址，可以为IPV4，也可以为IPV6地址
      */
    void set_peer_ip(const ip_address_t& ip);

    /** 设置对端端口号 */
	void set_peer_port(uint16_t port) { _peer_port = port; }

    /** 设置连接的允许的超时毫秒数 */
	void set_connect_timeout_milliseconds(uint32_t milli_seconds) { _milli_seconds = milli_seconds; }

    /***
      * 异步连接，不管是否能立即连接成功，都是立即返回
      * @return: 如果连接成功，则返回true，否则如果仍在连接过程中，则返回false
      * @exception: 连接错误，抛出CSyscallException异常
      */
    bool async_connect();

    /***
      * 超时连接
      * 如果不能立即连接成功，则等待由set_connect_timeout_milliseconds指定的时长，
      * 如果在这个时长内仍未连接成功，则立即返回
      * @exception: 连接出错或超时，抛出CSyscallException异常
      */
    void timed_connect();
    
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
      */
    ssize_t send(const char* buffer, size_t buffer_size);

    /***
      * 以超时方式接收数据，如果在指定的时间内未接收完，则返回
      * @milliseconds: 超时毫秒数
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
      * @buffer_size: 需要发送的字节数，返回实际已经发送了的字节数(不管成功还是失败或异常)
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

    /** 判断连接是否已经建立
      * @return: 如果连接已经建立，则返回true，否则返回false
      */
	bool is_connect_established() const;
    bool is_connect_establishing() const;

    /** 设置为已经连接状态，仅用于异步连接，其它情况什么都不做
      * async_connect可能返回正在连接中状态，当连接成功后，需要调用此函数来设置成已经连接状态，否则在调用close之前
      * 将一直处于正连接状态之中
      */
    void set_connected_state();   
    
    /** 得到当前已经连续的重连接次数 */
    volatile uint32_t get_reconnect_times() const;
    
    /** 得到连接超时毫秒值 */
    uint32_t get_connect_timeout_milliseconds() const;

public: // override
    /** 关闭连接 */
	virtual void close();

private:
    /** 连接成功之后被调用 */
    virtual void after_connect();
    /** connect之前被调用，可以(也可不)重写该方法，以在connect前做一些工作，如修改需要连接的IP等 */
    virtual bool before_connect();  
    /** 连接失败，当连接失败时会被调用 */
    virtual void on_connect_failure();

private:
    bool do_connect(int& fd, bool nonblock);    
    
protected:
    std::string do_to_string() const;

private:    
    uint16_t _peer_port;        /** 连接的对端端口号 */
    ip_address_t _peer_ip;      /** 连接的对端IP地址 */
    uint32_t _milli_seconds;    /** 连接超时的毫秒数 */
    void* _data_channel;
    uint8_t _connect_state;     /** 连接状态，1: 已经建立，2: 正在建立连接，0: 未连接 */
    atomic_t _reconnect_times;  /** 当前已经连续的重连接次数 */
};

NET_NAMESPACE_END
#endif // MOOON_NET_TCP_CLIENT_H
