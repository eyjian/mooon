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
#include "dispatcher/message.h"
#include "dispatcher_log.h"
DISPATCHER_NAMESPACE_BEGIN

message_t* create_message()
{
    char* message_buffer = new char[sizeof(message_t)];
    return reinterpret_cast<message_t*>(message_buffer);
}

message_t* create_stop_message()
{
    message_t* message = create_message();

    message->type = DISPATCH_STOP;
    message->length = 0;
    
    return message;
}

file_message_t* create_file_message(size_t file_size)
{
    char* message_buffer = new char[sizeof(message_t)+sizeof(file_message_t)];
    message_t* message = reinterpret_cast<message_t*>(message_buffer);

    message->type = DISPATCH_FILE;
    message->length = file_size;
        
    return reinterpret_cast<file_message_t*>(message->data);
}

buffer_message_t* create_buffer_message(size_t data_length)
{
    char* message_buffer = new char[sizeof(message_t)+data_length];
    message_t* message = reinterpret_cast<message_t*>(message_buffer);

    message->type = DISPATCH_BUFFER;
    message->length = data_length;

    return reinterpret_cast<buffer_message_t*>(message->data);
}

void destroy_message(message_t* message)
{
    char* message_buffer = reinterpret_cast<char*>(message);
    delete []message_buffer;
}

void destroy_file_message(file_message_t* file_messsage)
{
    char* message_buffer = reinterpret_cast<char*>(file_messsage)-sizeof(message_t);
    delete []reinterpret_cast<char*>(message_buffer);
}

void destroy_buffer_message(buffer_message_t* buffer_messsage)
{
    char* message_buffer = reinterpret_cast<char*>(buffer_messsage)-sizeof(message_t);
    delete []reinterpret_cast<char*>(message_buffer);
}

DISPATCHER_NAMESPACE_END
