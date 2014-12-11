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
 * Author: laocai_liu@qq.com or laocailiu@gmail.com
 */

#ifndef MOOON_PP_PACKET_HANDLER_H
#define MOOON_PP_PACKET_HANDLER_H

#include <server/server.h>
#include <server/connection.h>
#include <server/message_observer.h>
#include <server/packet_handler.h>
#include <net/inttypes.h>
#include <net/recv_machine.h>
#include "pp_log.h"

PP_NAMESPACE_BEGIN

class CppPakcetHandler : public server::IPacketHandler
{
public:
    CppPakcetHandler(server::IConnection* connection);
    ~CppPakcetHandler();

    bool on_header(const net::TCommonMessageHeader& header); // 解析出一个包头后被调用
    bool on_message(                                         // 每收到一点消息体时，都会被调用
               const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
             , size_t finished_size            // 已经接收到的大小
             , const char* buffer              // 当前收到的数据
             , size_t buffer_size);            // 当前收到的字节数

private:
    virtual void reset();
    virtual void on_connection_closed();
    virtual bool on_connection_timeout();
    virtual util::handle_result_t on_response_completed(server::Indicator& indicator);
    virtual util::handle_result_t on_handle_request(size_t data_size, server::Indicator& indicator);

private:
    // response buffer 连续内存大小
    int response_buffer_continue_available(void);

    // response buffer 空闲内存大小
    int response_buffer_all_available(void);

    // 整理 response buffer 的内存碎片
    void response_buffer_crunch(void);

private:
#define RECV_BODY_MAX 1024
    char _recv_body[RECV_BODY_MAX];
    int  _recv_ix;
    int _response_buffer_capability; // response buffer 最大值
    server::IConnection* _connection; // 建立的连接
    net::TCommonMessageHeader _request_header;
    net::CRecvMachine<net::TCommonMessageHeader, CppPakcetHandler> _recv_machine;
};

PP_NAMESPACE_END

#endif // MOOON_PP_PACKET_HANDLER_H

