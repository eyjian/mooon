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
#ifndef MOOON_DISPATCHER_LOG_H
#define MOOON_DISPATCHER_LOG_H
#include "mooon/dispatcher/dispatcher.h"
DISPATCHER_NAMESPACE_BEGIN
    
#define DISPATCHER_MODULE_NAME "DISPATCHER"

#define DISPATCHER_LOG_BIN(log, size)         __MYLOG_BIN(dispatcher::logger, log, size)
#define DISPATCHER_LOG_TRACE(format, ...)     __MYLOG_TRACE(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_FATAL(format, ...)     __MYLOG_FATAL(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_ERROR(format, ...)     __MYLOG_ERROR(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_WARN(format, ...)      __MYLOG_WARN(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_INFO(format, ...)      __MYLOG_INFO(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_DEBUG(format, ...)     __MYLOG_DEBUG(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_DETAIL(format, ...)    __MYLOG_DETAIL(dispatcher::logger, DISPATCHER_MODULE_NAME, format, ##__VA_ARGS__)

/***
  * 分发消息类型
  */
typedef enum
{
    DISPATCH_FILE,   /** 需要发送的是一个文件 */
    DISPATCH_BUFFER, /** 需要发送的是一个Buffer */
    DISPATCH_STOP    /** 停止Sender消息 */
}dispatch_type_t;

/***
  * 分发消息头
  */
typedef struct
{
    dispatch_type_t type; /** 分发消息类型 */
    size_t length;        /** 文件大小或content的字节数 */
    char data[0];
}message_t;

extern message_t* create_message();
extern message_t* create_stop_message();
extern void destroy_message(message_t* message);

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_LOG_H
