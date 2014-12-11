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
#include <string.h>
#include "http_event.h"
MY_NAMESPACE_BEGIN

CHttpEvent::CHttpEvent()
{
    memset(_on_name_value_pair_xxx, 0, sizeof(_on_name_value_pair_xxx));

    int i=3;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_3;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_4;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_5;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_6;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_7;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_8;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_9;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_10;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_11;
    _on_name_value_pair_xxx[i++] = &CHttpEvent::on_name_value_pair_12;
}

void CHttpEvent::reset()
{
	_header.reset();
}

bool CHttpEvent::on_head_end()
{
    return true;
}

void CHttpEvent::on_error(const char* errmsg)
{
    MYLOG_DEBUG("Http parsing error: %s.\n", errmsg);
}

bool CHttpEvent::on_method(const char* begin, const char* end)
{
    MYLOG_DEBUG("Http method: %.*s.\n", (int)(end-begin), begin);

    int len = end-begin;
    switch (len)
    {
    case 3:
        if (0 == strncasecmp(begin, "GET", len))
        {
            _header.set_method(CHttpHeader::hm_get);
            return true;
        }
    case 4:
        if (0 == strncasecmp(begin, "POST", len))
        {
            _header.set_method(CHttpHeader::hm_post);
            return true;
        }
        break;
    }
    
    return false;
}

bool CHttpEvent::on_url(const char* begin, const char* end)
{
    MYLOG_DEBUG("URL: %.*s.\n", (int)(end-begin), begin);
    _header.set_url(begin, end-begin);
    return true;
}

bool CHttpEvent::on_version(const char* begin, const char* end)
{
    MYLOG_DEBUG("Version: %.*s.\n", (int)(end-begin), begin);
    return true;
}

bool CHttpEvent::on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end)
{
    MYLOG_DEBUG("Name: %.*s, Value: %.*s.\n", (int)(name_end-name_begin), name_begin, (int)(value_end-value_begin), value_begin);
    size_t name_len = name_end - name_begin;
    size_t value_len = value_end - value_begin;
    
    if ((name_len < 3) || (name_len>sizeof(_on_name_value_pair_xxx)/sizeof(on_name_value_pair_xxx)))
    {
        MYLOG_ERROR("Error NV - %.*s: %.*s.\n", (int)name_len, name_begin, (int)value_len, value_begin);
        return false;
    }
    
    if (NULL == _on_name_value_pair_xxx[name_len]) return true;
    
    return (this->*_on_name_value_pair_xxx[name_len])(name_begin, name_len, value_begin, value_len);
}

bool CHttpEvent::on_name_value_pair_3(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_4(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    if (0 == strncasecmp(name_begin, "host", name_len))
    {
        _header.set_domain_name(value_begin, value_len);
    }
    return true;
}

bool CHttpEvent::on_name_value_pair_5(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_6(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_7(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_8(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_9(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_10(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
	if (0 == strncasecmp(name_begin, "Connection", name_len))
	{		
		if (0 == strncasecmp(value_begin, "Keep-Alive", value_len))
		{
			_header.set_keep_alive(true);
		}
		else
		{
			_header.set_keep_alive(false);
		}
	}
	
    return true;
}

bool CHttpEvent::on_name_value_pair_11(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

bool CHttpEvent::on_name_value_pair_12(const char* name_begin, int name_len, const char* value_begin, int value_len)
{
    return true;
}

MY_NAMESPACE_END
