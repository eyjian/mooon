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
#ifndef HTTP_REPLY_HANDLER_H
#define HTTP_REPLY_HANDLER_H
#include <dispatcher/dispatcher.h>
#include <http_parser/http_parser.h>
#include "http_event.h"
MOOON_NAMESPACE_BEGIN

class CHttpReplyHandler: public dispatcher::IReplyHandler
{
public:
	CHttpReplyHandler();
	~CHttpReplyHandler();

	uint32_t get_num_success() const
	{
		return _num_success;
	}

	uint32_t get_num_failure() const
	{
		return _num_failure;
	}

	uint64_t get_bytes_recv() const
	{
		return _bytes_recv;
	}

	uint64_t get_bytes_send() const
	{
		return _bytes_send;
	}

private:
	virtual void attach(dispatcher::ISender* sender);
	virtual char* get_buffer();
	virtual size_t get_buffer_length() const;
	virtual size_t get_buffer_offset() const;
	virtual void send_progress(size_t total, size_t finished, size_t current);
	virtual void sender_connected();
	virtual void sender_connect_failure();
	virtual void send_completed();
	virtual void sender_closed();
	virtual util::handle_result_t handle_reply(size_t data_size);

private:
	void send_request();
	bool is_finish() const;
	void inc_num_success();
	void inc_num_failure();
	void inc_bytes_recv(uint32_t bytes);
	void inc_bytes_send(uint32_t bytes);

private:
	dispatcher::ISender* _sender;
	CHttpEvent _http_event;
	http_parser::IHttpParser* _http_parser;

private:
	char _buffer[1024];

private:
	bool _is_success; // 是否已经成功响应
	uint32_t _num_success;
	uint32_t _num_failure;
	uint64_t _bytes_recv;
	uint64_t _bytes_send;
};

MOOON_NAMESPACE_END
#endif // HTTP_REPLY_HANDLER_H
