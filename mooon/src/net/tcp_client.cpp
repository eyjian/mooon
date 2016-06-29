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
#include "mooon/net/tcp_client.h"
#include "data_channel.h"
#include "mooon/net/utils.h"
#include <sstream>
#define CONNECT_UNESTABLISHED 0
#define CONNECT_ESTABLISHED   1
#define CONNECT_ESTABLISHING  2
NET_NAMESPACE_BEGIN

CTcpClient::CTcpClient()
	:_peer_port(0)
	,_milli_seconds(0)
	,_connect_state(CONNECT_UNESTABLISHED)
{
	_data_channel = new CDataChannel;
    atomic_set(&_reconnect_times, 0);
}

CTcpClient::~CTcpClient()
{
	delete (CDataChannel *)_data_channel;
}

std::string CTcpClient::to_string() const
{
    return std::string("waiter:://") + do_to_string();
}

std::string CTcpClient::do_to_string() const
{
    std::stringstream id;
    id << get_fd()
       << "@"
       << _peer_ip.to_string()
       << ":"
       << _peer_port;

    return id.str();
}

bool CTcpClient::is_ipv6() const
{
    return _peer_ip.is_ipv6();
}

uint16_t CTcpClient::get_peer_port() const
{
    return _peer_port;
}

const ip_address_t& CTcpClient::get_peer_ip() const
{
    return _peer_ip;
}

void CTcpClient::set_peer(const ip_node_t& ip_node)
{
    _peer_ip = ip_node.ip;
    _peer_port = ip_node.port;
}

void CTcpClient::set_peer(const ipv4_node_t& ip_node)
{    
    _peer_ip = ip_node.ip;
    _peer_port = ip_node.port;
}

void CTcpClient::set_peer(const ipv6_node_t& ip_node)
{
    _peer_ip = (uint32_t*)ip_node.ip;
    _peer_port = ip_node.port;
}

void CTcpClient::set_peer_ip(const ip_address_t& ip)
{
    _peer_ip = ip;
}

void CTcpClient::close()
{	    
    if (is_connect_establishing())
    {
        // 不调用before_close()
        on_connect_failure();
        CEpollable::do_close();
            
    }
    else
    {
        // 会调用before_close()
	    CEpollable::close();        
    }

    _connect_state = CONNECT_UNESTABLISHED;
}

void CTcpClient::after_connect()
{
    // 子类可以选择去做点事
}

bool CTcpClient::before_connect()
{
    // 子类可以选择去做点事
    return true;
}

void CTcpClient::on_connect_failure()
{
    // 子类可以选择去做点事
}

bool CTcpClient::do_connect(int& fd, bool nonblock)
{   
    // 方便在连接之前做一些处理
    if (!before_connect()) return false;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "socket");

    if (nonblock)
        net::set_nonblock(fd, true);

    socklen_t addr_length;
    struct sockaddr* peer_addr;
    struct sockaddr_in peer_addr_in;
    struct sockaddr_in6 peer_addr_in6;    
            
    const uint32_t* ip_data = _peer_ip.get_address_data();
    if (_peer_ip.is_ipv6())
    {
        addr_length = sizeof(struct sockaddr_in6);
        peer_addr = (struct sockaddr*)&peer_addr_in6;        
        peer_addr_in6.sin6_family = AF_INET6;
        peer_addr_in6.sin6_port = htons(_peer_port);
        memcpy(&peer_addr_in6.sin6_addr, ip_data, addr_length);
    }
    else
    {
        addr_length = sizeof(struct sockaddr_in);
        peer_addr = (struct sockaddr*)&peer_addr_in;
	    peer_addr_in.sin_family = AF_INET;
        peer_addr_in.sin_port = htons(_peer_port);
        peer_addr_in.sin_addr.s_addr = ip_data[0];
        memset(peer_addr_in.sin_zero, 0, sizeof(peer_addr_in.sin_zero));
    }	
    
    return (0 == connect(fd, peer_addr, addr_length)) || (EISCONN == errno);
}

bool CTcpClient::is_connect_established() const 
{ 
    return CONNECT_ESTABLISHED == _connect_state; 
}

bool CTcpClient::is_connect_establishing() const 
{ 
    return CONNECT_ESTABLISHING == _connect_state; 
}

void CTcpClient::set_connected_state()
{
    if (CONNECT_ESTABLISHING == _connect_state)
    {
        _connect_state = CONNECT_ESTABLISHED;
        atomic_set(&_reconnect_times, 0); // 一旦连接成功，就将重连接次数清零
        after_connect();
    }
}

volatile uint32_t CTcpClient::get_reconnect_times() const
{
    return atomic_read(&_reconnect_times);
}

uint32_t CTcpClient::get_connect_timeout_milliseconds() const
{
    return _milli_seconds;
}

bool CTcpClient::async_connect()
{
    int fd = -1;    
    atomic_inc(&_reconnect_times); // 重连接次数增1，如果连接成功则减1
    
    if (!do_connect(fd, true))
    {
        if (errno != EINPROGRESS)    
        {
            on_connect_failure(); // 连接失败
            THROW_SYSCALL_EXCEPTION(NULL, errno, "connect");
        }
    }
    
    set_fd(fd);
    ((CDataChannel *)_data_channel)->attach(fd);
    _connect_state = (EINPROGRESS == errno)? CONNECT_ESTABLISHING: CONNECT_ESTABLISHED;
    
    // 连接已经成功
    if (CONNECT_ESTABLISHED == _connect_state) 
    {
        atomic_set(&_reconnect_times, 0);
        after_connect();
    }

    // 连接还在进行中
    return errno != EINPROGRESS;
}

void CTcpClient::timed_connect()
{                 	
    int fd = -1;
    atomic_inc(&_reconnect_times); // 重连接次数增1，如果连接成功则减1
    
    do
    {    
	    try
	    {
            // 超时连接需要先设置为非阻塞
            bool nonblock = _milli_seconds > 0;                   
            if (do_connect(fd, nonblock))
                break; // 一次性连接成功了
            
            // 连接出错，不能继续
            if ((0 == _milli_seconds) || (errno != EINPROGRESS))
                THROW_SYSCALL_EXCEPTION(NULL, errno, "connect");

            // 异步连接中，使用poll超时探测
            if (!CUtils::timed_poll(fd, POLLIN | POLLOUT | POLLERR, _milli_seconds))
                THROW_SYSCALL_EXCEPTION(NULL, ETIMEDOUT, "poll");

		    int errcode = 0;
		    socklen_t errcode_length = sizeof(errcode);
		    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &errcode, &errcode_length))
		        THROW_SYSCALL_EXCEPTION(NULL, ETIMEDOUT, "getsockopt");
		    if (errcode != 0)
		        THROW_SYSCALL_EXCEPTION(NULL, errcode, "connect");

            // 能够走到这里，肯定是_milli_seconds > 0
            net::set_nonblock(fd, false);
	    }
	    catch (sys::CSyscallException& ex)
	    {
            on_connect_failure(); // 连接失败
		    ::close(fd);
		    throw;
	    }
    } while(false);

    // 关联fd
    set_fd(fd);
    ((CDataChannel *)_data_channel)->attach(fd);
	_connect_state = CONNECT_ESTABLISHED;
    atomic_set(&_reconnect_times, 0); // 一旦连接成功，就将重连接次数清零
    after_connect();
}

ssize_t CTcpClient::receive(char* buffer, size_t buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->receive(buffer, buffer_size); 
}

ssize_t CTcpClient::send(const char* buffer, size_t buffer_size)
{ 
	return ((CDataChannel *)_data_channel)->send(buffer, buffer_size); 
}

ssize_t CTcpClient::timed_receive(char* buffer, size_t buffer_size, uint32_t milliseconds)
{
    return ((CDataChannel *)_data_channel)->timed_receive(buffer, buffer_size, milliseconds); 
}

ssize_t CTcpClient::timed_send(const char* buffer, size_t buffer_size, uint32_t milliseconds)
{
    return ((CDataChannel *)_data_channel)->timed_send(buffer, buffer_size, milliseconds); 
}

bool CTcpClient::full_receive(char* buffer, size_t& buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->full_receive(buffer, buffer_size); 
}

void CTcpClient::full_send(const char* buffer, size_t& buffer_size)
{ 
	((CDataChannel *)_data_channel)->full_send(buffer, buffer_size); 
}

ssize_t CTcpClient::send_file(int file_fd, off_t *offset, size_t count)
{
    return ((CDataChannel *)_data_channel)->send_file(file_fd, offset, count); 
}

void CTcpClient::full_send_file(int file_fd, off_t *offset, size_t& count)
{
    ((CDataChannel *)_data_channel)->full_send_file(file_fd, offset, count); 
}

bool CTcpClient::full_map_tofile(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->full_map_tofile(file_fd, size, offset); 
}

bool CTcpClient::full_write_tofile(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->full_write_tofile(file_fd, size, offset); 
}

ssize_t CTcpClient::readv(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->readv(iov, iovcnt);
}

ssize_t CTcpClient::writev(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->writev(iov, iovcnt);
}

NET_NAMESPACE_END
