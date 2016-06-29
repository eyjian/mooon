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
#ifndef MOOON_NET_IP_ADDRESS_H
#define MOOON_NET_IP_ADDRESS_H
#include <set>
#include <vector>
#include <utility> // std::pair
#include "mooon/net/config.h"
#include "mooon/sys/syscall_exception.h"
NET_NAMESPACE_BEGIN

/** IP地址，兼容IPV4和IPV6 */
class ip_address_t
{
public:    
    /** 构造一个127.0.0.1地址，不抛出异常 */
    ip_address_t();
    
    /** 构造一个IPV4地址，不抛出异常 */
    ip_address_t(uint32_t ipv4);

    /***
      * 构造一个IPV6地址
      * @exception: 如参数为NULL，则抛出utils::CException异常
      */
    ip_address_t(const uint32_t* ipv6) throw (utils::CException);
    
    /***
      * 构造一个IP地址，可以为IPV4，也可为IPV6
      * @ip: 字符串IP地址，如果为NULL，则构造一个0.0.0.0的IPV4地址
      * @exception: 如果为无效IP地址，则抛出utils::CException异常
      */
    ip_address_t(const char* ip) throw (utils::CException);

    /** 拷贝构造，不抛出异常 */
    ip_address_t(const ip_address_t& ip);

    /** 判断是否为IPV6地址，不抛出异常 */
    bool is_ipv6() const;

    /** 转换成字符串，不抛出异常 */
    std::string to_string() const;

    /** 取IPV4地址 */
    uint32_t to_ipv4() const;

    /** 取IPV6地址 */
    const uint32_t* to_ipv6() const;

    /** 得到地址数据的有效字节数 */
    size_t get_address_data_length() const;

    /** 得到地址数据 */
    const uint32_t* get_address_data() const;    

    /** 判断是否为0.0.0.0地址 */
    bool is_zero_address() const;

    /** 判断是否为广播地址 */
    bool is_broadcast_address() const;

public: // 赋值和比较操作
    ip_address_t& operator =(uint32_t ipv4);
    ip_address_t& operator =(const uint32_t* ipv6) throw (utils::CException);
    ip_address_t& operator =(const char* ip);
    ip_address_t& operator =(const ip_address_t& other);
    bool operator <(const ip_address_t& other) const;
    bool operator >(const ip_address_t& other) const;
    bool operator ==(const ip_address_t& other) const;    
    
private:
    void from_string(const char* ip) throw (utils::CException);
    
private:
    bool _is_ipv6;
    uint32_t _ip_data[4];
};

inline ip_address_t::ip_address_t()
    :_is_ipv6(false)
{
    _ip_data[0] = 0;
    _ip_data[1] = 0;
    _ip_data[2] = 0;
    _ip_data[3] = 0;
}

inline ip_address_t::ip_address_t(uint32_t ipv4)
    :_is_ipv6(false)
{
    _ip_data[0] = ipv4;
    _ip_data[1] = 0;
    _ip_data[2] = 0;
    _ip_data[3] = 0;
}

inline ip_address_t::ip_address_t(const uint32_t* ipv6) throw (utils::CException)
    :_is_ipv6(true)
{
    if (NULL == ipv6)
        THROW_EXCEPTION(NULL, EINVAL);

    memcpy(_ip_data, ipv6, sizeof(_ip_data));
}

inline ip_address_t::ip_address_t(const char* ip) throw (utils::CException)
{
    from_string(ip);
}

inline ip_address_t::ip_address_t(const ip_address_t& other)
{
    _is_ipv6 = other.is_ipv6();
    memcpy(_ip_data, other.get_address_data(), sizeof(_ip_data));
}

inline ip_address_t& ip_address_t::operator =(uint32_t ipv4)
{
    _is_ipv6 = false;
    _ip_data[0] =  ipv4;
    _ip_data[1] = 0;
    _ip_data[2] = 0;
    _ip_data[3] = 0;
    return *this;
}

inline ip_address_t& ip_address_t::operator =(const uint32_t* ipv6) throw (utils::CException)
{
    if (NULL == ipv6)
        THROW_EXCEPTION(NULL, EINVAL);

    memcpy(_ip_data, ipv6, sizeof(_ip_data));
    return *this;
}

inline ip_address_t& ip_address_t::operator =(const char* ip)
{
    from_string(ip);
    return *this;
}

inline ip_address_t& ip_address_t::operator =(const ip_address_t& other)
{
    _is_ipv6 = other.is_ipv6();
    memcpy(_ip_data, other.get_address_data(), sizeof(_ip_data));
    return *this;
}

inline bool ip_address_t::operator <(const ip_address_t& other) const
{
    const uint32_t* ip_data = other.get_address_data();
    
    return _is_ipv6? (-1 == memcmp(_ip_data, ip_data, sizeof(_ip_data))): (_ip_data[0] < ip_data[0]);
}

inline bool ip_address_t::operator >(const ip_address_t& other) const
{
    const uint32_t* ip_data = other.get_address_data();
    return _is_ipv6? (1 == memcmp(_ip_data, ip_data, sizeof(_ip_data))): (_ip_data[0] > ip_data[0]);
}

inline bool ip_address_t::operator ==(const ip_address_t& other) const
{
    const uint32_t* other_ip_data = other.get_address_data();

    if (_is_ipv6 && other.is_ipv6())
    {
        return 0 == memcmp(_ip_data, other_ip_data, sizeof(_ip_data));
    }
    else if (!_is_ipv6 && !other.is_ipv6())
    {
        return _ip_data[0] == other_ip_data[0];
    }
    else
    {
        return false;
    }
}

inline bool ip_address_t::is_ipv6() const
{
    return _is_ipv6;
}

/** ip_address_t类型的Hash函数 */
typedef struct
{
    uint64_t operator()(const ip_address_t* ip_address) const
    {
        const uint32_t* address_data = ip_address->get_address_data();
        return ip_address->is_ipv6()? (address_data[1] + address_data[3]): address_data[0];
    }
}ip_address_hasher;

/** ip_address_t类型的比较函数 */
typedef struct
{
    bool operator()(const ip_address_t* lhs, const ip_address_t* rhs) const
    {
        return *lhs == *rhs;
    }
}ip_address_comparser;

/** IP地址集合 */
typedef std::set<ip_address_t> ip_address_set_t;

/** ip_address_t类型指针数组 */
typedef std::vector<ip_address_t> ip_address_array_t;

/** IP和端口对 */
typedef std::pair<ip_address_t, port_t> ip_port_pair_t;

/** IP和端口对类型指针数组 */
typedef std::vector<ip_port_pair_t> ip_port_pair_array_t;

NET_NAMESPACE_END
#endif // MOOON_NET_IP_ADDRESS_H
