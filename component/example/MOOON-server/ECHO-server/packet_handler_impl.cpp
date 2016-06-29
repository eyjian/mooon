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
#include <mooon/sys/utils.h>
#include "packet_handler_impl.h"

CPakcetHandlerImpl::CPakcetHandlerImpl(mooon::server::IConnection* connection)
    :_connection(connection)
{
    _request_context.request_size = mooon::sys::CUtils::get_page_size();
    _request_context.request_buffer = new char[_request_context.request_size];
}

CPakcetHandlerImpl::~CPakcetHandlerImpl()
{
    delete []_request_context.request_buffer;
}

void CPakcetHandlerImpl::reset()
{
    _response_context.response_size = 0;
    _response_context.response_offset = 0;
}

mooon::utils::handle_result_t CPakcetHandlerImpl::on_handle_request(size_t data_size, mooon::server::Indicator& indicator)
{
    _response_context.response_size = data_size;
    _response_context.response_buffer = _request_context.request_buffer;
    return mooon::utils::handle_finish; /** finish表示请求已经接收完成，可进入响应过程 */
}

mooon::utils::handle_result_t CPakcetHandlerImpl::on_response_completed(mooon::server::Indicator& indicator)
{
    // 如果收到quit指令，则关闭连接
    return 0 == strncmp(_request_context.request_buffer, "quit", sizeof("quit")-1)
        ? mooon::utils::handle_finish /** finish表示可关闭连接 */
        : mooon::utils::handle_continue; /** continue表示连接保持，不要关闭 */
}
