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
#ifndef MOOON_NET_CONFIG_H
#define MOOON_NET_CONFIG_H
#include <vector>
#include <mooon/utils/config.h>

// 定义名字空间宏
#define NET_NAMESPACE_BEGIN namespace mooon { namespace net {
#define NET_NAMESPACE_END                   }}
#define NET_NAMESPACE_USE using namespace mooon::net;

// 断言宏
#define NET_ASSERT assert

NET_NAMESPACE_BEGIN

/** 端口类型 */
typedef uint16_t port_t;
typedef std::vector<uint32_t> int_ip_array_t; /** IP地址数组 */
typedef std::vector<std::string> string_ip_array_t; /** IP地址数组 */
typedef std::vector<std::pair<std::string, std::string> > eth_ip_array_t; /** 网卡名和IP对数组 */

/***
  * IP地址类型定义
  */
typedef enum TIPType
{
	IP_TYPE_4 = 0, // IP是一个IPV4地址
	IP_TYPE_6 = 1  // IP是一个IPV6地址
}ip_type_t;

NET_NAMESPACE_END
#endif // MOOON_NET_CONFIG_H
