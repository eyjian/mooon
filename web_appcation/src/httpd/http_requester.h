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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef HTTP_REQUESTER_H
#define HTTP_REQUESTER_H
#include "http_event.h"
#include "gtf/protocol_parser.h"
#include "http_parser/http_parser.h"
MY_NAMESPACE_BEGIN

class CHttpRequester: public IProtocolParser
{
public:
	CHttpRequester();
	~CHttpRequester();
	bool get_keep_alive() const { return _http_event.get_keep_alive(); }
	const CHttpEvent* get_http_event() const { return &_http_event; }

private:
	virtual void reset();
	virtual uint32_t get_ip() const { return _ip; }
	virtual void set_ip(uint32_t ip) { _ip = ip; }
	virtual uint16_t get_port() const { return _port; }
	virtual void set_port(uint16_t port) { _port = port; }

	virtual util::TReturnResult parse(const char* buffer, int buffer_length);
	virtual uint32_t get_buffer_length() const;
	virtual char* get_buffer();

private:
	uint32_t _ip;
	uint16_t _port;
    
private:
	char* _buffer;
	int _buffer_offset;
	int _buffer_length;    
	CHttpEvent _http_event;
	IHttpParser* _http_parser;    
};

MY_NAMESPACE_END
#endif // HTTP_REQUESTER_H
