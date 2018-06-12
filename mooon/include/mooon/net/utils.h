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
#ifndef MOOON_NET_UTILS_H
#define MOOON_NET_UTILS_H
#include "mooon/net/config.h"
#include "mooon/sys/syscall_exception.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
NET_NAMESPACE_BEGIN

/***
  * 与网络相关的工具类
  */
class CUtils
{
public:    
    /** 判断是否为小字节序，如果是返回true，否则返回false */
	static bool is_little_endian()
	{
	#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN)
	#   if (__BYTE_ORDER == __LITTLE_ENDIAN)
	        return true;
	#   else
	        return false;
	#   endif
	#else
	    union
	    {
	        uint16_t a;
	        uint8_t  b[2];
	    }x;

	    // 小字节序：数据的低字节放在低地址处
	    // 不管何平台，编译器总是保证：&b[0] < &b[1]

	    x.a = 0x0102; // 01为高字节，02为低字节
	    return (x.b[0] == 0x02) && (x.b[1] == 0x01);
	#endif // LITTLE_ENDIAN
	}

    /***
      * 反转字节
      * @source: 源字节
      * @result: 反转后的字节
      * @length: 需要反转的长度
      */
    static void reverse_bytes(const void* source, void* result, size_t length);
    
    /***
      * 反转字节
      * @source: 源字节
      * @return: 反转后的字节
      */
    template <typename DataType>
    static DataType reverse_bytes(const DataType& source)
    {
        DataType result = 0;
        reverse_bytes(&source, &result, sizeof(result));
        return result;
    }

    /***
      * 反转字节
      * @source: 源字节
      * @result: 反转后的字节
      */
    template <typename DataType>
    static void reverse_bytes(const DataType* source, DataType* result)
    {
        CUtils::reverse_bytes(source, result, sizeof(DataType));
    }

    /***
      * 将源数据从主机字节序转换成网络字节序
      * @source: 需要转换的主机字节序源数据
      * @result: 存放转换后的网络字节序结果数据
      * @length: 需要转换的字节长度
      */
    static void host2net(const void* source, void* result, size_t length);

    /***
      * 将源数据从网络字节序转换成主机字节序
      * @source: 需要转换的网络字节序源数据
      * @result: 存放转换后的主机字节序结果数据
      * @length: 需要转换的字节长度
      */
    static void net2host(const void* source, void* result, size_t length);

    /***
      * 将源数据从主机字节序转换成网络字节序
      * @source: 需要转换的主机字节序源数据
      * @result: 存放转换后的网络字节序结果数据
      */
    template <typename DataType>
    static void host2net(const DataType& source, DataType& result)
    {
        host2net(&source, &result, sizeof(result));        
    }

    /***
      * 将源数据从主机字节序转换成网络字节序
      * @source: 需要转换的主机字节序源数据
      * @return: 存放转换后的网络字节序结果数据
      */
    template <typename DataType>
    static DataType host2net(const DataType& source)
    {
        DataType result;
        host2net(&source, &result, sizeof(result));   
        return result;
    }

    /***
      * 将源数据从网络字节序转换成主机字节序
      * @source: 需要转换的网络字节序源数据
      * @result: 存放转换后的主机字节序结果数据
      */
    template <typename DataType>
    static void net2host(const DataType& source, DataType& result)
    {
        CUtils::host2net<DataType>(source, result);
    }    

    /***
      * 将源数据从网络字节序转换成主机字节序
      * @source: 需要转换的网络字节序源数据
      * @return: 存放转换后的主机字节序结果数据
      */
    template <typename DataType>
    static DataType net2host(const DataType& source)
    {
        return CUtils::host2net<DataType>(source);
    } 

    /** 判断给定的字符串是否为主机名或域名 */
    static bool is_host_name(const char* str);
    
    /** 判断给定的字符串是否为一个IPV4或IPV6地址
      * @return: 如果给定的字符串是一个IPV4或IPV6地址，则返回true，否则返回false
      */
    static bool is_valid_ip(const char* str);

    /** 判断给定的字符串是否为一个IPV4地址
      * @return: 如果给定的字符串是一个IPV4地址，则返回true，否则返回false
      */
    static bool is_valid_ipv4(const char* str);

    /** 判断给定的字符串是否为一个IPV6地址
      * @return: 如果给定的字符串是一个IPV6地址，则返回true，否则返回false
      */
    static bool is_valid_ipv6(const char* str);

    /***
      * 根据主机名得到一个IP地址
      * @hostname: 主机名
      * @ip_array: 存放IP的数组
      * @errinfo: 用来保存错误信息
      * @return: 如果成功返回true，否则返回false
      * @exception: 无异常抛出
      */
    static bool get_ip_address(const char* hostname, string_ip_array_t& ip_array, std::string& errinfo);
    
    /** 得到网卡名和对应的IP
      * @eth_ip_array: 用于保存所有获取到的IP地址
      * @exception: 如果发生错误，抛出CSyscallException异常
      */
    static void get_ethx_ip(eth_ip_array_t& eth_ip_array);

    /** 根据网卡名得到绑定在该网卡上的所有IP地址
      * @ethx: 网卡名，如：eth0，如果为NULL，则表示所有网卡
      * @ip_array: 用于保存所有获取到的IP地址
      * @exception: 如果发生错误，抛出CSyscallException异常
      */
    static void get_ethx_ip(const char* ethx, string_ip_array_t& ip_array);   

    /**
      * 将网卡名转换成IP地址，如果source已经是IP则保持不变
      * 如果一块网卡上绑定有多个IP，则只取get_ethx_ip()函数返回的第一个IP
      */
    static std::string transform_ip(const std::string& source);

    /** 根据整数类型的IP，得到字符串类型的IP地址
      * @ip: 整数类型的IP
      * @return: 字符串类型的IP地址
      */
    static std::string ipv4_tostring(uint32_t ipv4);
    static std::string ipv6_tostring(const uint32_t* ipv6);    

    /** 将一个字符串转换成IPV4地址类型
      * @source: 需要转换的字符串
      * @ipv4: 存储转换后的IPV4地址
      * @return: 转换成功返回true，否则返回false
      */
    static bool string_toipv4(const char* source, uint32_t& ipv4);

    /** 将一个字符串转换成IPV6地址类型
      * @source: 需要转换的字符串
      * @ipv6: 存储转换后的IPV6地址，必须为连续的16字节，如: uint32_t[4]
      * @return: 转换成功返回true，否则返回false
      */
    static bool string_toipv6(const char* source, uint32_t* ipv6);
    
    /** 判断传入的字符串是否为接口名，如：eth0等
      * @return: 如果str是接口名，则返回true，否则返回false
      */
    static bool is_ethx(const char* str);

    /** 判断传入的字符串是否为广播地址
      * @str: IP地址字符串，可以为NULL
      * @return: 如果str为广播地址，则返回true，否则返回false
      */
    static bool is_broadcast_address(const char* str);

    /** 超时POLL单个fd对象
      * @fd: 被POLL的单个fd，注意不是fd数组
      * @events_requested: 请求监控的事件，常用值为POLLIN或POLLOUT
      * @milliseconds: 阻塞的毫米数，总是保证等待这个时长，即使被中断
      * @events_returned: 用来保存返回的事件，如果为NULL，则无事件返回，通过检测返回事件，可以明确是哪个事件发生了
      * @return: 超时返回false，有事件返回true
      * @exception: 网络错误，则抛出CSyscallException异常
      */
    static bool timed_poll(int fd, int events_requested, int milliseconds, int* events_returned=NULL);
};

extern std::string to_string(const struct in_addr& sin_addr);
extern std::string to_string(const struct in6_addr& sin_addr);
extern std::string to_string(const sockaddr_in& addr);
extern std::string to_string(const sockaddr_in6& addr);
extern std::string ip2string(uint32_t ip);
extern uint32_t string2ipv4(const char* ip);
extern uint32_t string2ipv4(const std::string& ip);

extern bool is_loop_ipv4(uint32_t ip);
extern bool is_loop_ipv4(const char* ip);
extern bool is_loop_ipv4(const std::string& ip);

// ip 为网络字节序值
extern bool is_local_ipv4(uint32_t ip);
extern bool is_local_ipv4(const char* ip);
extern bool is_local_ipv4(const std::string& ip);

NET_NAMESPACE_END
#endif // MOOON_NET_UTILS_H
