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

#include <sys/util.h>
#include "pp_reply_handler.h"
#include "pp_msg.h"

PP_NAMESPACE_BEGIN

CppReplyHandler::CppReplyHandler()
   : _sender(NULL)
    ,_offset(0)
    ,_recv_machine(this)
{
    _length = sys::CUtil::get_page_size();
    _buffer = new char[_length];    
    _msg_body = new char[_length];
}

CppReplyHandler::~CppReplyHandler()
{
    delete []_msg_body;
    delete []_buffer;
}

bool CppReplyHandler::on_header(const net::TCommonMessageHeader& header)
{
    return true;
}

bool CppReplyHandler::on_message(
        const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
      , size_t finished_size            // 已经接收到的大小
      , const char* buffer              // 当前收到的数据
      , size_t buffer_size)
{
    MOOON_ASSERT(_length >= header.size);
    MOOON_ASSERT(header.size >= finished_size + buffer_size);

    memcpy(_msg_body + finished_size, buffer, buffer_size);
    if (header.size == finished_size + buffer_size)
    {
        dispatcher::buffer_message_t* buffer_message;

        //{
        //    std::string msg(_msg_body, header.size);

        //    PP_LOG_INFO("[CppReplyHandler::on_message] msg is %s", msg.c_str());
        //}

        // ping pong 数据返回
        buffer_message = create_pp_message(_msg_body, header.size);
        _sender->push_message(buffer_message);
    }

    return true;
}

void CppReplyHandler::attach(dispatcher::ISender* sender)
{
    _sender = sender;
}

void CppReplyHandler::sender_closed()
{
    PP_LOG_INFO("CppReplyHandler::sender_closed");
}

void CppReplyHandler::sender_connect_failure()
{
    PP_LOG_ERROR("CppReplyHandler::sender_connect_failure");
}

char* CppReplyHandler::get_buffer()
{
    return _buffer;
}

size_t CppReplyHandler::get_buffer_length() const
{
    return _length;
}

size_t CppReplyHandler::get_buffer_offset() const
{
    return _offset;
}

util::handle_result_t CppReplyHandler::handle_reply(size_t data_size)
{
    // work会调用CppReplyHandler::on_message和CppReplyHandler::on_header
    util::handle_result_t handle_result = _recv_machine.work(_buffer + _offset
                                                            , data_size);
    if (util::handle_finish == handle_result)
    	return util::handle_continue;
    else
    {
        PP_LOG_ERROR("CppReplyHandler::handle_reply %d", handle_result);
        return handle_result;
    }
}

PP_NAMESPACE_END

