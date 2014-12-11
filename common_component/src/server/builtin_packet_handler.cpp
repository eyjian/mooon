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
#include "builtin_packet_handler.h"
#include "log.h"
SERVER_NAMESPACE_BEGIN

CBuiltinPacketHandler::CBuiltinPacketHandler(IConnection* connection, IMessageObserver* message_observer)
 :_connection(connection)
 ,_message_observer(message_observer)
 ,_recv_machine(this)
{
    _request_context.request_size = sizeof(_request_header);
    _request_context.request_buffer = reinterpret_cast<char*>(&_request_header);
}

CBuiltinPacketHandler::~CBuiltinPacketHandler()
{
    delete _message_observer;
    reset();
}

bool CBuiltinPacketHandler::on_header(const net::TCommonMessageHeader& header)
{
    SERVER_LOG_TRACE("enter %s.\n", __FUNCTION__);

    memcpy(&_request_header, &header, sizeof(_request_header));
    uint32_t size = _request_header.size.to_int();

    _request_context.request_size = size;
    _request_context.request_offset = 0;
    if (0 == size)
        _request_context.request_buffer = NULL;
    else
        _request_context.request_buffer = new char[size];

    return true;
}

bool CBuiltinPacketHandler::on_message(
        const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
      , size_t finished_size            // 已经接收到的大小
      , const char* buffer              // 当前收到的数据
      , size_t buffer_size)
{
    SERVER_LOG_TRACE("enter %s.\n", __FUNCTION__);

    if (finished_size+buffer_size == header.size)
    {
        // 完整包体
        if (_response_context.response_buffer != NULL)
        {
            SERVER_LOG_ERROR("%s is not NULL.\n", _response_context.to_string().c_str());
        }

        // 防止on_message()抛异常
        const char* request_buffer = _request_context.request_buffer;
        _request_context.request_buffer = NULL;

        if (!_message_observer->on_message(header
                                        , request_buffer
                                        , &_response_context.response_buffer
                                        , &_response_context.response_size))
        {
            SERVER_LOG_DEBUG("%s on_message ERROR.\n", _connection->str().c_str());
            return false;
        }

        if ((0 == _response_context.response_size)
         && (_response_context.response_buffer != NULL))
        {
            SERVER_LOG_WARN("%s.\n", _response_context.to_string().c_str());
        }
        else
        {
            SERVER_LOG_DEBUG("%s.\n", _response_context.to_string().c_str());
        }
    }

    return true;
}

void CBuiltinPacketHandler::reset()
{
    // 复位请求参数
    char* request_buffer= reinterpret_cast<char*>(&_request_header);
    if (_request_context.request_buffer != request_buffer)
        delete []_request_context.request_buffer;
    _request_context.request_buffer = reinterpret_cast<char*>(&_request_header);
    _request_context.request_size = sizeof(_request_header);
    _request_context.request_offset = 0;

    // 复位响应参数
    delete []_response_context.response_buffer;
    _response_context.response_buffer = NULL;
    _response_context.response_size = 0;
    _response_context.response_offset = 0;
}

util::handle_result_t CBuiltinPacketHandler::on_handle_request(size_t data_size, Indicator& indicator)
{
    SERVER_LOG_TRACE("enter %s.\n", __FUNCTION__);

    // 注意：work会调用CBuiltinPacketHandler::on_message和CBuiltinPacketHandler::on_header
    util::handle_result_t handle_result = _recv_machine.work(_request_context.request_buffer
                                                            +_request_context.request_offset
                                                            , data_size);

    // 成功无响应，则返回util::handle_continue
    return ((util::handle_finish == handle_result)
         && (NULL == _response_context.response_buffer || 0 == _response_context.response_size))
         ? handle_result = util::handle_continue
         : handle_result;
}

void CBuiltinPacketHandler::on_connection_closed()
{
    _message_observer->on_connection_closed();
}

bool CBuiltinPacketHandler::on_connection_timeout()
{
    return _message_observer->on_connection_timeout();
}

util::handle_result_t CBuiltinPacketHandler::on_response_completed(Indicator& indicator)
{
    util::handle_result_t handle_result = _message_observer->on_response_completed();

    if (util::handle_continue == handle_result)
        return util::handle_continue;
    if (util::handle_error == handle_result)
        return util::handle_error;

    return util::handle_close;
}

SERVER_NAMESPACE_END
