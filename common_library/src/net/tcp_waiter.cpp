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
#include "data_channel.h"
#include "mooon/net/tcp_waiter.h"
#include <sstream>
NET_NAMESPACE_BEGIN

CTcpWaiter::CTcpWaiter()
    :_self_port(0)
    ,_peer_port(0)
{
	_data_channel = new CDataChannel;
}

CTcpWaiter::~CTcpWaiter()
{
	delete (CDataChannel *)_data_channel;
}

std::string CTcpWaiter::to_string() const
{
    return std::string("waiter:://") + do_to_string();
}

std::string CTcpWaiter::do_to_string() const
{
    std::stringstream id;
    id << get_fd()
       << "@"
       << _peer_ip.to_string()
       << ":"
       << _peer_port
       << "->"
       << _self_ip.to_string()
       << ":"
       << _self_port;

    return id.str();
}

void CTcpWaiter::set_self(const ip_address_t& self_ip, port_t self_port)
{
    _self_ip = self_ip;
    _self_port = self_port;
}

void CTcpWaiter::attach(int fd, const ip_address_t& peer_ip, port_t peer_port)
{ 
    set_fd(fd);
    _peer_ip = peer_ip;
    _peer_port = peer_port;  
    
    ((CDataChannel *)_data_channel)->attach(fd);
}

ssize_t CTcpWaiter::receive(char* buffer, size_t buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->receive(buffer, buffer_size); 
}

ssize_t CTcpWaiter::send(const char* buffer, size_t buffer_size)
{ 
	return ((CDataChannel *)_data_channel)->send(buffer, buffer_size); 
}

ssize_t CTcpWaiter::timed_receive(char* buffer, size_t buffer_size, uint32_t milliseconds)
{
    return ((CDataChannel *)_data_channel)->timed_receive(buffer, buffer_size, milliseconds); 
}

ssize_t CTcpWaiter::timed_send(const char* buffer, size_t buffer_size, uint32_t milliseconds)
{
    return ((CDataChannel *)_data_channel)->timed_send(buffer, buffer_size, milliseconds); 
}

bool CTcpWaiter::full_receive(char* buffer, size_t& buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->full_receive(buffer, buffer_size); 
}

void CTcpWaiter::full_send(const char* buffer, size_t& buffer_size)
{ 
	((CDataChannel *)_data_channel)->full_send(buffer, buffer_size); 
}

ssize_t CTcpWaiter::send_file(int file_fd, off_t *offset, size_t count)
{
    return ((CDataChannel *)_data_channel)->send_file(file_fd, offset, count); 
}

void CTcpWaiter::full_send_file(int file_fd, off_t *offset, size_t& count)
{
    ((CDataChannel *)_data_channel)->full_send_file(file_fd, offset, count); 
}

bool CTcpWaiter::full_map_tofile(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->full_map_tofile(file_fd, size, offset); 
}

bool CTcpWaiter::full_write_tofile(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->full_write_tofile(file_fd, size, offset); 
}

ssize_t CTcpWaiter::readv(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->readv(iov, iovcnt);
}

ssize_t CTcpWaiter::writev(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->writev(iov, iovcnt);
}

NET_NAMESPACE_END
