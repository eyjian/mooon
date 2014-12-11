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
#include "http_requester.h"
MY_NAMESPACE_BEGIN

CHttpRequester::CHttpRequester()
{    
    _buffer_length = 2048;
    _buffer = new char[_buffer_length];
	
    _http_parser = my::create_http_parser();
    _http_parser->set_http_event(&_http_event);

	reset();
}

CHttpRequester::~CHttpRequester()
{
	delete []_buffer;
    my::destroy_http_parser(_http_parser);
}

void CHttpRequester::reset()
{
	_ip = 0;
	_port = 0;
	_buffer[0] = '\0';
    _buffer_offset = 0;
	_http_event.reset();
	_http_parser->reset();	
}

util::TReturnResult CHttpRequester::parse(const char* buffer, int buffer_length)
{
	_buffer_offset += buffer_length;
    return _http_parser->parse(buffer);
}

uint32_t CHttpRequester::get_buffer_length() const
{
    return _buffer_length-_buffer_offset;
}

char* CHttpRequester::get_buffer()
{
    return _buffer+_buffer_offset;
}

MY_NAMESPACE_END
