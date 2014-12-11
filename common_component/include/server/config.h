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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef MOOON_SERVER_CONFIG_H
#define MOOON_SERVER_CONFIG_H
#include <sys/log.h>
#include <net/ip_address.h>

/***
 * 编译开关宏
 */
#define ENABLE_SET_SERVER_THREAD_NAME 1 /** 是否允许设置server线程名 */

/***
  * server模块的名字空间名称
  */
#define SERVER_NAMESPACE_BEGIN  namespace mooon { namespace server {
#define SERVER_NAMESPACE_END                    }}
#define SERVER_NAMESPACE_USE using mooon::server;

//////////////////////////////////////////////////////////////////////////
SERVER_NAMESPACE_BEGIN

/***
  * 配置回调接口
  */
class CALLBACK_INTERFACE IConfig
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IConfig() {}

    /** 得到epoll大小 */
    virtual uint32_t get_epoll_size() const { return 10000; }

    /** 得到框架的工作线程个数 */
    virtual uint16_t get_thread_number() const { return 1; }

    /** 得到每个线程的连接池大小 */
    virtual uint32_t get_connection_pool_size() const { return 10000; }

    /** 连接超时秒数 */
    virtual uint32_t get_connection_timeout_seconds() const { return 10; }

    /** 得到epool等待超时毫秒数 */
    virtual uint32_t get_epoll_timeout_milliseconds() const { return 2000; }

    /** 得到监听参数 */    
    virtual const net::ip_port_pair_array_t& get_listen_parameter() const = 0;

    /** 得到每个线程的接管队列的大小 */
    virtual uint32_t get_takeover_queue_size() const { return 100; }
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_CONFIG_H
