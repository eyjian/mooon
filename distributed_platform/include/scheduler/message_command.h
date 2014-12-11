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
#ifndef MOOON_SCHEDULER_MESSAGE_COMMAND_H
#define MOOON_SCHEDULER_MESSAGE_COMMAND_H
#include <scheduler/config.h>
SCHED_NAMESPACE_BEGIN

enum
{
    REQUEST_COMMAND         = 0,
    RESPONSE_COMMAND        = 1,
    TIMER_COMMAND           = 2,
    CREATE_SESSION_COMMAND  = 3,
    DESTROY_SESSION_COMMAND = 4
};

/***
  * Service消息命令字
  */
typedef enum TServiceMessageCommand
{
	SERVICE_COMMAND_MIN     = 101,

	SERVICE_REQUEST         = SERVICE_COMMAND_MIN + REQUEST_COMMAND, // 发给Service的请求消息
	SERVICE_RESPONSE        = SERVICE_COMMAND_MIN + RESPONSE_COMMAND, // 发给Service的应答消息
	SERVICE_TIMER           = SERVICE_COMMAND_MIN + TIMER_COMMAND, // 发给Service的定时器消息
	SERVICE_CREATE_SESSION  = SERVICE_COMMAND_MIN + CREATE_SESSION_COMMAND, // 发给Service的创建Session消息
	SERVICE_DESTROY_SESSION = SERVICE_COMMAND_MIN + DESTROY_SESSION_COMMAND, // 发给Service的销毁Session消息

	SERVICE_COMMAND_MAX     = SERVICE_COMMAND_MIN + 4
}service_message_command_t;

/***
  * Session消息命令字
  */
typedef enum TSessionMessageCommand
{
	SESSION_COMMAND_MIN     = 201,

	SESSION_REQUEST         = SESSION_COMMAND_MIN + REQUEST_COMMAND, // 发给Session的请求消息
	SESSION_RESPONSE        = SESSION_COMMAND_MIN + RESPONSE_COMMAND, // 发给Session的应答消息
	SESSION_TIMER           = SESSION_COMMAND_MIN + TIMER_COMMAND, // 发给Session的定时器消息
	SESSION_CREATE_SESSION  = SESSION_COMMAND_MIN + CREATE_SESSION_COMMAND, // 发给Session的创建子Session消息
	SESSION_DESTROY_SESSION = SESSION_COMMAND_MIN + DESTROY_SESSION_COMMAND, // 发给Session的销毁子Session消息

	SESSION_COMMAND_MAX     = SESSION_COMMAND_MIN + 4
}session_message_command_t;

inline bool is_service_message(uint32_t command)
{
	return command >= SERVICE_COMMAND_MIN
        && command <= SERVICE_COMMAND_MAX;
}

inline bool is_session_message(uint32_t command)
{
	return command >= SESSION_COMMAND_MIN
	    && command <= SESSION_COMMAND_MAX;
}

inline bool is_response_message(uint32_t command)
{
	return SERVICE_RESPONSE == command
	    || SESSION_RESPONSE == command;
}

inline bool is_request_message(uint32_t command)
{
	return !is_response_message(command);
}

/***
  * 将命令转化成一个可以作为数组下标的值
  * @command 命令字
  * @return 失败返回-1，成功返回一个可以作为数组下标的值
  */
inline int command2index(uint32_t command)
{
    if (is_service_message(command))
    {
        return command - SERVICE_COMMAND_MIN;
    }
    else if (is_session_message(command))
    {
        // 和Service连着
        return command - SESSION_COMMAND_MIN;
    }

    return -1;
}

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_MESSAGE_COMMAND_H
