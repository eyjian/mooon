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
#ifndef MOOON_SERVER_BUILTIN_PACKET_HANDLER_H
#define MOOON_SERVER_BUILTIN_PACKET_HANDLER_H
#include <mooon/server/connection.h>
#include <mooon/server/message_observer.h>
#include <mooon/server/packet_handler.h>
#include <mooon/net/inttypes.h>
#include <mooon/net/recv_machine.h>
SERVER_NAMESPACE_BEGIN

/***
  * 内置的包处理器，提供基于net::TCommonMessageHeader
  * 格式的通用解决方案，以提升server组件的易用性
  */
class CBuiltinPacketHandler: public IPacketHandler
{
public:
    CBuiltinPacketHandler(IConnection* connection, IMessageObserver* message_observer);
    ~CBuiltinPacketHandler();

    bool on_header(const net::TCommonMessageHeader& header); // 解析出一个包头后被调用
    bool on_message(                                         // 每收到一点消息体时，都会被调用
               const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
             , size_t finished_size            // 已经接收到的大小
             , const char* buffer              // 当前收到的数据
             , size_t buffer_size);            // 当前收到的字节数

private:
    virtual void reset();
    virtual utils::handle_result_t on_handle_request(size_t data_size, Indicator& indicator);
    virtual void on_connection_closed();
    virtual bool on_connection_timeout();
    virtual utils::handle_result_t on_response_completed(Indicator& indicator);

private:
    IConnection* _connection;
    IMessageObserver* _message_observer;
    net::TCommonMessageHeader _request_header;
    net::CRecvMachine<net::TCommonMessageHeader, CBuiltinPacketHandler> _recv_machine;
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_BUILTIN_PACKET_HANDLER_H
