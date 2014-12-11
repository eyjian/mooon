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
#include <sstream>

#include <util/args_parser.h>

#include <dispatcher/message.h>
#include <http_parser/http_parser.h>

#include "counter.h"
#include "http_event.h"
#include "http_reply_handler.h"

INTEGER_ARG_DECLARE(uint32_t, nr);

MOOON_NAMESPACE_BEGIN

CHttpReplyHandler::CHttpReplyHandler()
 :_sender(NULL)
 ,_is_success(false)
 ,_num_success(0)
 ,_num_failure(0)
 ,_bytes_recv(0)
 ,_bytes_send(0)
{
	_http_parser = http_parser::create(false);
	_http_parser->set_http_event(&_http_event);
}

CHttpReplyHandler::~CHttpReplyHandler()
{
	http_parser::destroy(_http_parser);
}

void CHttpReplyHandler::attach(dispatcher::ISender* sender)
{
	_sender = sender;
}

char* CHttpReplyHandler::get_buffer()
{
	return _buffer;
}

size_t CHttpReplyHandler::get_buffer_length() const
{
	return sizeof(_buffer);
}

size_t CHttpReplyHandler::get_buffer_offset() const
{
	return 0;
}

void CHttpReplyHandler::send_progress(size_t total, size_t finished, size_t current)
{
	_bytes_send += static_cast<uint64_t>(current);
}

void CHttpReplyHandler::sender_connected()
{
	send_request();
}

void CHttpReplyHandler::sender_connect_failure()
{
	sender_closed();
}

void CHttpReplyHandler::send_completed()
{

}

void CHttpReplyHandler::sender_closed()
{
	if (!_is_success)
	{
		inc_num_failure();
		_is_success = false;
	}
}

util::handle_result_t CHttpReplyHandler::handle_reply(size_t data_size)
{
	_bytes_recv += static_cast<uint64_t>(data_size);

	util::handle_result_t hr = _http_parser->parse(_buffer);
	if (hr != util::handle_finish)
	{
		return hr;
	}

	CHttpEvent* http_event = static_cast<CHttpEvent*>(_http_parser->get_http_event());
	if (http_event->get_code() != 200)
	{
		return util::handle_error;
	}

	inc_num_success();
	if (is_finish())
	{
		return util::handle_close;
	}

	send_request(); // 继续发送请求
	return util::handle_finish;
}

void CHttpReplyHandler::send_request()
{
	const std::string& http_req = CCounter::get_singleton()->get_http_req();
	dispatcher::buffer_message_t* msg = dispatcher::create_buffer_message(http_req.size());
	strcpy(msg->data, http_req.c_str());
	_sender->push_message(msg);
}

bool CHttpReplyHandler::is_finish() const
{
	return _num_success + _num_failure == ArgsParser::nr->get_value();
}

void CHttpReplyHandler::inc_num_success()
{
	++_num_success;
	_is_success = true;

	if (is_finish())
	{
		_sender->set_reconnect_times(0);
		CCounter::get_singleton()->inc_num_sender_finished();
	}

//    printf("num_success: %u, num_failure=%u\n", _num_success, _num_failure);
}

void CHttpReplyHandler::inc_num_failure()
{
	++_num_failure;

	if (is_finish())
	{
		_sender->set_reconnect_times(0);
		CCounter::get_singleton()->inc_num_sender_finished();
	}

//    printf("num_success: %u, num_failure=%u\n", _num_success, _num_failure);
}

void CHttpReplyHandler::inc_bytes_recv(uint32_t bytes)
{
	_bytes_recv += bytes;
}

void CHttpReplyHandler::inc_bytes_send(uint32_t bytes)
{
	_bytes_send += bytes;
}

MOOON_NAMESPACE_END
