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
#ifndef MOOON_AGENT_MESSAGE_H
#define MOOON_AGENT_MESSAGE_H
#include <mooon/agent/message_command.h>
#include <mooon/net/inttypes.h>
AGENT_NAMESPACE_BEGIN
#pragma pack(4) // 网络消息按4字节对齐

/***
  * 简单的心跳消息，仅一个消息头
  */
typedef struct TSimpleHeartbeatMessage
{
    net::TCommonMessageHeader header;
    char app_data[0];
}simple_heartbeat_message_t;

/***
  * 上报消息
  */
typedef struct TReportMessage
{
    net::TCommonMessageHeader header;
    char data[0]; /** 需要上报的内容 */
}report_message_t;

#pragma pack()
AGENT_NAMESPACE_END
#endif // MOOON_AGENT_MESSAGE_H
