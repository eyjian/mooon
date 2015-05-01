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
#ifndef MOOON_NET_SEND_MACHINE_H
#define MOOON_NET_SEND_MACHINE_H
#include <mooon/net/config.h>
NET_NAMESPACE_BEGIN

template <class Connector>
class CSendMachine
{
public:
    CSendMachine(Connector* connector);
    bool is_finish() const;
    utils::handle_result_t continue_send();
    utils::handle_result_t send(const char* msg, size_t msg_size);
    void reset(bool delete_message);
    
private:
    Connector* _connector;
    
private:
    const char* _message;
    const char* _cursor;
    size_t _remain_size;    
};

template <class Connector>
CSendMachine<Connector>::CSendMachine(Connector* connector)
 :_connector(connector) 
{
    reset(false);
}

// 当前消息是否已经发送完
template <class Connector>
bool CSendMachine<Connector>::is_finish() const
{
    return 0 == _remain_size;
}

// 发送消息，可能是一个消息的第一次发送，也可能是一个消息的非第一次发送
template <class Connector>
utils::handle_result_t CSendMachine<Connector>::continue_send()
{
    ssize_t bytes_sent = _connector->send(_cursor, _remain_size);
    if (bytes_sent > -1)
    {
        _cursor += bytes_sent;
        _remain_size -= bytes_sent;
    }
    
    return is_finish() 
         ? utils::handle_finish 
         : utils::handle_continue;
}

// 发送消息，总是一个消息的第一次发送
// 参数说明：
// msg - 需要发送的消息
// msg_size - 需要发送的消息字节数
template <class Connector>
utils::handle_result_t CSendMachine<Connector>::send(const char* msg, size_t msg_size)
{
    _message = msg;
    _cursor = msg;
    _remain_size = msg_size;
    
    return continue_send();
}

template <class Connector>
void CSendMachine<Connector>::reset(bool delete_message)
{
    if (delete_message)
        delete []_message;

    _message = NULL;
    _cursor = NULL;
    _remain_size = 0;
}

NET_NAMESPACE_END
#endif // MOOON_NET_SEND_MACHINE_H
