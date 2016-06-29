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

#ifndef MOOON_PP_REPLY_HANDLER_H
#define MOOON_PP_REPLY_HANDLER_H

#include <net/recv_machine.h>
#include <net/inttypes.h>
#include <dispatcher/dispatcher.h>
#include "pp_log.h"

PP_NAMESPACE_BEGIN


class CppReplyHandler : public dispatcher::IReplyHandler
{
public:
    CppReplyHandler();
    ~CppReplyHandler();

public:
    // 由 CRecvMachine 回调
    bool on_header(const net::TCommonMessageHeader& header); // 解析出一个包头后被调用
    bool on_message(                                         // 每收到一点消息体时，都会被调用
               const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
             , size_t finished_size            // 已经接收到的大小
             , const char* buffer              // 当前收到的数据
             , size_t buffer_size);            // 当前收到的字节数

private:
    virtual void attach(dispatcher::ISender* sender);

    virtual void sender_closed();
    virtual void sender_connect_failure();

    virtual char* get_buffer();
    virtual size_t get_buffer_length() const;
    virtual size_t get_buffer_offset() const;
    virtual util::handle_result_t handle_reply(size_t data_size);

private:
    dispatcher::ISender* _sender;
    size_t _length;
    size_t _offset;
    char* _buffer; // 网络数据接收缓存

    char* _msg_body; // 接收的消息体都存放在此缓存中
    net::CRecvMachine<net::TCommonMessageHeader, CppReplyHandler> _recv_machine;
};

PP_NAMESPACE_END

#endif
