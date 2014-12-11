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
 *
 * 什么是Session？它是一个有状态的远程对象。
 */
#ifndef MOOON_SCHEDULER_MESSAGE_H
#define MOOON_SCHEDULER_MESSAGE_H
#include <scheduler/config.h>
#include <net/inttypes.h>
#include <scheduler/message_command.h>

// 为何要做IPV4和IPV6区分？
// 是因为如果只需要支持IPV4，则一个IP地址只需要用一个4字节表示，
// 这样源和目的IP地址共节省了24字节；
// 支持IPV6时，同时兼容IPV4，但对于IPV4多浪费24字节；
// 源和目标IP，要么都为IPV4，要么都为IPV6，不允许交叉出现。
#ifdef IPV6_SUPPORTED
    #define IP_BYTES (4*4) // IPV6的地址长度
#else
    #define IP_BYTES 4     // IPV4的地址长度
#endif

////////////////////////////////////////////////////////////////////////////////
SCHED_NAMESPACE_BEGIN

/***
  * 常量定义
  */
enum
{
	INVALID_SERVICE_ID = 0,
	INVALID_SESSION_ID = 0,  // 无效的SessionId
	SERVICE_ID_MAX = 100,    // 最大的Service ID值，取值从1开始
	SESSION_ID_MAX = 10000   // 最大的Session ID值，取值从1开始
};

// 按4字节对齐
#pragma pack(4)

/***
  * 分布式消息Flags结构
  * 为什么Flags要单独定义成一个struct，
  * 是因为nuint32_t类型不支持位表达方式，
  * 所以使用struct做一层转换，以达到相同的目的
  */
typedef struct TDistributedMessageFlags
{
    // 使用union，方便操作
    union Flags
    {
        uint32_t flags;
        struct TFlagsBits
        {
            uint32_t ip_type:1;      // IP地址类型，取值为net::IP_TYPE_4或net::IP_TYPE_6
            uint32_t reserved:31;    // 保留用的位
        }flags_bits;
    }flags;

    TDistributedMessageFlags(uint32_t v)
     :flags(v)
    {
    }
}distribted_message_flags_t;

/***
  * 分布式消息头结构
  */
typedef struct TDistributedMessage
{
	net::common_message_header header;     // 消息头
    nuint32_t flags;                       // 标志字段

    nuint32_t source_ip[IP_BYTES];         // 消息源的IP地址，如果是IPV4地址，则N值为1，否则为4
    nuint32_t destination_ip[IP_BYTES];    // 消息目的地的IP地址，如果是IPV4地址，则N值为1，否则为4

    nuint16_t source_port;                 // 消息源的端口号
    nuint16_t destination_port;            // 消息目的地的端口号

    nuint32_t source_service_id;           // destination_Service ID
    nuint32_t destination_service_id;      // 消息目的地的Service ID

    nuint32_t source_session_id;           // destination_Session ID
    nuint32_t destination_session_id;      // 消息目的地的Session ID

    nuint32_t source_sequence_number;      // 序列号，从0开始，依次递增，直到重来，用于解决类似于TCP中的timed_wait问题
    nuint32_t destination_sequence_number; // 序列号，从0开始，依次递增，直到重来，用于解决类似于TCP中的timed_wait问题

    nuint32_t source_thread_affinity;      // 线程亲和值，为的是和线程建立绑定关系
    nuint32_t destination_thread_affinity; // 线程亲和值，为的是和线程建立绑定关系

    char data[0];                          // 消息内容

    std::string to_string() const;
}distribted_message_t;

#pragma pack()

inline bool is_valid_service(uint32_t service_id)
{
	return service_id > 0
		&& service_id <= SERVICE_ID_MAX;
}

inline bool is_valid_session(uint32_t session_id)
{
	return session_id > 0
		&& session_id <= SESSION_ID_MAX;
}

////////////////////////////////////////////////////////////////////////////////
SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_MESSAGE_H
