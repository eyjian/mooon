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
#include "http_header.h"
MY_NAMESPACE_BEGIN

CHttpHeader::CHttpHeader()
{
	reset();
}

void CHttpHeader::reset()
{
	_keep_alive = false;
	_domain_name = NULL;
	_domain_name_length = 0;
	_url = NULL;
	_url_length = 0;
}

const char* CHttpHeader::get_domain_name(uint16_t& length) const
{
    length = _domain_name_length;
    return _domain_name;
}

void CHttpHeader::set_domain_name(const char* begin, uint16_t length)
{
    _domain_name = (char *)begin;
    _domain_name_length = length;
}

const char* CHttpHeader::get_url(uint16_t& length) const
{
    length = _url_length;
    return _url;
}

void CHttpHeader::set_url(const char* begin, uint16_t length)
{
    _url = (char *)begin;
    _url_length = length;
}

MY_NAMESPACE_END
