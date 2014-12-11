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
 * 上行消息定义：agent -> center
 * 下行消息定义：center -> agent
 *
 */
#ifndef MOOON_AGENT_MESSAGE_COMMAND_H
#define MOOON_AGENT_MESSAGE_COMMAND_H
#include <agent/config.h>
AGENT_NAMESPACE_BEGIN

/***
  * 上行消息命令字
  */
typedef enum TUplinkMessageCommand
{
    U_SIMPLE_HEARTBEAT_MESSAGE = 1, /** 简单心跳消息 */
    U_REPORT_MESSAGE           = 2  /** 上报消息 */
}uplink_message_command_t;

/***
  * 下行消息命令字，由ICommandProcessor处理
  */
typedef enum TDownlinkMessageCommand
{

}downlink_message_command_t;

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_MESSAGE_COMMAND_H
