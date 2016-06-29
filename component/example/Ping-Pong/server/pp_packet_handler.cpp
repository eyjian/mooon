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

#include <iostream>
#include <sys/util.h>
#include "pp_packet_handler.h"
#include "statistics.h"

PP_NAMESPACE_BEGIN

CppPakcetHandler::CppPakcetHandler(server::IConnection* connection)
    :_connection(connection), _recv_machine(this)
{
    _request_context.request_size = sys::CUtil::get_page_size();
    _request_context.request_buffer = new char[_request_context.request_size];
    _response_buffer_capability = _request_context.request_size;
    _response_context.response_buffer = new char[_response_buffer_capability];
    reset();
}

CppPakcetHandler::~CppPakcetHandler()
{
    delete []_response_context.response_buffer;
    delete []_request_context.request_buffer;
}

void CppPakcetHandler::on_connection_closed()
{
}

bool CppPakcetHandler::on_connection_timeout()
{
    return true;
}

bool CppPakcetHandler::on_header(const net::TCommonMessageHeader& header)
{
    _recv_ix = 0;

    return true;
}

bool CppPakcetHandler::on_message(
        const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
      , size_t finished_size            // 已经接收到的大小
      , const char* buffer              // 当前收到的数据
      , size_t buffer_size)
{
    const static int MSG_HEAD_SIZE = sizeof(net::TCommonMessageHeader);

    int pack_size = header.size + MSG_HEAD_SIZE;
    //std::cout << "CppPakcetHandler::on_message|finished_size=" << finished_size << "|pack_size=" << pack_size << std::endl;

    if (finished_size+buffer_size >= header.size)
    {
        MOOON_ASSERT(finished_size+buffer_size == header.size);
        if (pack_size > response_buffer_continue_available())
        {
            if (pack_size <= response_buffer_all_available())
            {
                response_buffer_crunch();
            }
            else
            {
                PP_LOG_ERROR("[CppPakcetHandler::on_message][response buffer is too small error] \
                             finished_size=%d, pack_size=%d\n"
                            , finished_size, pack_size);
                return false;
            }
        }
        memcpy(_response_context.response_buffer + _response_context.response_size, &header, MSG_HEAD_SIZE);
         _response_context.response_size += MSG_HEAD_SIZE;
        memcpy(_response_context.response_buffer + _response_context.response_size, _recv_body, finished_size);
        _response_context.response_size += finished_size;
        memcpy(_response_context.response_buffer + _response_context.response_size, buffer, buffer_size);
        _response_context.response_size += buffer_size;
    }
    else
    {
        MOOON_ASSERT(_recv_ix + buffer_size <= RECV_BODY_MAX);
        memcpy(_recv_body + _recv_ix, buffer, buffer_size);
        _recv_ix += buffer_size;
    }

    return true;
}

bool on_connection_timeout()
{
    return true;
}

void CppPakcetHandler::reset()
{
    _response_context.response_size = 0;
    _response_context.response_offset = 0;
    _recv_ix = 0;
}

util::handle_result_t CppPakcetHandler::on_handle_request(size_t data_size, server::Indicator& indicator)
{
    util::handle_result_t handle_result = _recv_machine.work(_request_context.request_buffer
                                                                +_request_context.request_offset
                                                                , data_size);

    // 成功无响应，则返回util::handle_continue
    return ((util::handle_finish == handle_result)
         && (NULL == _response_context.response_buffer || 0 == _response_context.response_size))
         ? handle_result = util::handle_continue
         : handle_result;
}

util::handle_result_t CppPakcetHandler::on_response_completed(server::Indicator& indicator)
{
    CStatistics::get_singleton()->inc_pp_msg_count();

    return util::handle_continue;
}

int CppPakcetHandler::response_buffer_continue_available(void)
{
    MOOON_ASSERT(_response_buffer_capability >= _response_context.response_size);
    return (_response_buffer_capability - _response_context.response_size);
}

int CppPakcetHandler::response_buffer_all_available(void)
{
    MOOON_ASSERT(_response_buffer_capability >= _response_context.response_size);
    return (_response_buffer_capability - (_response_context.response_size - _response_context.response_offset));
}

void CppPakcetHandler::response_buffer_crunch(void)
{
    size_t size;
    size_t offset;

    size = _response_context.response_size;
    offset = _response_context.response_offset;
    memmove(_response_context.response_buffer, _response_context.response_buffer + offset, size - offset);
    _response_context.response_size = size - offset;
   _response_context.response_offset = 0;
}

PP_NAMESPACE_END

