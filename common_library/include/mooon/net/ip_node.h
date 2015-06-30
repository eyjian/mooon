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
#ifndef MOOON_NET_IP_NODE_H
#define MOOON_NET_IP_NODE_H
#include <mooon/net/ip_address.h>
NET_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// ipv4_node_t

/***
  * IPV4结点类型
  */
typedef struct ipv4_node_t
{
    uint16_t port;  /** 端口号 */
    uint32_t ip;    /** IPV4地址 */
    
    /* 构造、赋值和比较函数 */
    ipv4_node_t();
    ipv4_node_t(uint16_t new_port, uint32_t new_ip);
    ipv4_node_t(const ipv4_node_t& other);
    ipv4_node_t& operator =(const ipv4_node_t& other);
    bool operator ==(const ipv4_node_t& other) const;
}ipv4_node_t;

inline ipv4_node_t::ipv4_node_t()
    :port(0)
    ,ip(0)
{
}

inline ipv4_node_t::ipv4_node_t(uint16_t new_port, uint32_t new_ip)
    :port(new_port)
    ,ip(new_ip)
{        
}

inline ipv4_node_t::ipv4_node_t(const ipv4_node_t& other)
    :port(other.port)
    ,ip(other.ip)
{        
}

inline ipv4_node_t& ipv4_node_t::operator =(const ipv4_node_t& other)
{
    port = other.port;
    ip = other.ip;
    return *this;
}

inline bool ipv4_node_t::operator ==(const ipv4_node_t& other) const
{
    return (port == other.port) && (ip == other.ip);
}

//////////////////////////////////////////////////////////////////////////
// ipv6_node_t

/***
  * IPV6结点类型
  */
typedef struct ipv6_node_t
{
    uint16_t port;  /** 端口号 */
    uint32_t ip[4]; /** IPV6地址 */

    /* 构造、赋值和比较函数 */
    ipv6_node_t();
    ipv6_node_t(uint16_t new_port, const uint32_t* new_ip);
    ipv6_node_t(const ipv6_node_t& other);
    ipv6_node_t& operator =(const ipv6_node_t& other);
    bool operator ==(const ipv6_node_t& other) const;
}ipv6_node_t;

inline ipv6_node_t::ipv6_node_t()
    :port(0)
{
    memset(ip, 0, sizeof(ip));
}

inline ipv6_node_t::ipv6_node_t(uint16_t new_port, const uint32_t* new_ip)
    :port(new_port)
{
    memcpy(ip, new_ip, sizeof(ip));
}

inline ipv6_node_t::ipv6_node_t(const ipv6_node_t& other)
    :port(other.port)
{
    memcpy(ip, other.ip, sizeof(ip));
}

inline ipv6_node_t& ipv6_node_t::operator =(const ipv6_node_t& other)
{
    port = other.port;
    memcpy(ip, other.ip, sizeof(ip));
    return *this;
} 

inline bool ipv6_node_t::operator ==(const ipv6_node_t& other) const
{
    return (port == other.port) && (0 == memcmp(ip, other.ip, sizeof(ip)));
}

//////////////////////////////////////////////////////////////////////////
// ip_node_t
typedef struct ip_node_t
{
    uint16_t port; /** 端口号 */
    ip_address_t ip; /** IP地址 */

    /* 构造、赋值和比较函数 */
    ip_node_t();
    ip_node_t(uint16_t new_port, const ip_address_t& new_ip);
    ip_node_t(const ip_node_t& other);
    ip_node_t& operator =(const ip_node_t& other);
    bool operator ==(const ip_node_t& other);       
}ip_node_t;

inline ip_node_t::ip_node_t()
    :port(0)
{
}

inline ip_node_t::ip_node_t(uint16_t new_port, const ip_address_t& new_ip)
    :port(new_port)
    ,ip(new_ip)
{
}

inline ip_node_t::ip_node_t(const ip_node_t& other)
    :port(other.port)
    ,ip(other.ip)
{
}

inline ip_node_t& ip_node_t::operator =(const ip_node_t& other)
{
    port = other.port;
    ip = other.ip;
    return *this;
}

inline bool ip_node_t::operator ==(const ip_node_t& other)
{
    return ip == other.ip && port == other.port;
}

//////////////////////////////////////////////////////////////////////////
// Hash helper

/** IPV4的hash函数 */
typedef struct
{
    uint64_t operator()(const ipv4_node_t& ip_node) const
    {
		return ip_node.ip + ip_node.port;
	}
}ipv4_node_hasher;

/** IPV4的比较函数 */
typedef struct
{
    bool operator()(const ipv4_node_t& lhs, const ipv4_node_t& rhs) const
    {
		return (lhs.port == rhs.port) && (lhs.ip == rhs.ip);
	}
}ipv4_node_comparer;

/** IPV6的hash函数 */
typedef struct
{
    uint64_t operator()(const ipv6_node_t& ip_node) const
    {
		return ip_node.ip[1] + ip_node.ip[3] + ip_node.port;
	}
}ipv6_node_hasher;

/** IPV6的比较函数 */
typedef struct
{
    bool operator()(const ipv6_node_t& lhs, const ipv6_node_t& rhs) const
    {
		return (lhs.port == rhs.port) && (0 == memcmp(lhs.ip, rhs.ip, sizeof(ipv6_node_t)));
	}
}ipv6_node_comparer;

/** IP的hash函数 */
typedef struct
{
    uint64_t operator()(const ip_node_t& ip_node) const
    {
        const uint32_t* ip_data = ip_node.ip.get_address_data();

        return ip_data[0]
        + ip_data[1]
        + ip_data[2]
        + ip_data[3]
        + ip_node.port;
    }
}ip_node_hasher;

/** IP的比较函数 */
typedef struct
{
    bool operator()(const ip_node_t& lhs, const ip_node_t& rhs) const
    {
        return (lhs.port == rhs.port) && (lhs.ip == rhs.ip);
    }
}ip_node_comparer;

NET_NAMESPACE_END
#endif // MOOON_NET_IP_NODE_H
