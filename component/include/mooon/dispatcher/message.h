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
#ifndef MOOON_DISPATCHER_MESSAGE_H
#define MOOON_DISPATCHER_MESSAGE_H
#include <mooon/dispatcher/config.h>
DISPATCHER_NAMESPACE_BEGIN

/***
  * 分发文件类型消息结构
  */
typedef struct 
{
    int fd;           /** 需要发送的文件描述符 */
    off_t offset;     /** 文件偏移，从文件哪个位置开始发送 */
}file_message_t;

/***
  * 分发Buffer类型消息结构
  */
typedef struct
{
    char data[0];     /** 需要发送的消息 */
}buffer_message_t;

extern file_message_t* create_file_message(size_t file_size);
extern buffer_message_t* create_buffer_message(size_t data_length);

extern void destroy_file_message(file_message_t* file_messsage);
extern void destroy_buffer_message(buffer_message_t* buffer_messsage);

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_MESSAGE_H
