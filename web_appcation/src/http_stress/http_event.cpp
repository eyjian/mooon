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
#include "http_event.h"

#include <util/string_util.h>
#include "counter.h"
MOOON_NAMESPACE_BEGIN

CHttpEvent::CHttpEvent()
    :_content_length(-1)
{
}

CHttpEvent::~CHttpEvent()
{
	CCounter::get_singleton()->inc_num_sender_finished();
}

int CHttpEvent::get_code() const
{
	return _code;
}

int CHttpEvent::get_content_length() const
{
    return _content_length;
}

void CHttpEvent::reset()
{
    _content_length = -1;
}

bool CHttpEvent::on_head_end()
{
    return true;
}

void CHttpEvent::on_error(const char* errmsg)  
{
    MYLOG_ERROR("HTTP ERROR: %s.\n", errmsg);
}

bool CHttpEvent::on_method(const char* begin, const char* end)
{
    return true;
}

bool CHttpEvent::on_url(const char* begin, const char* end)
{
    return true;
}

bool CHttpEvent::on_version(const char* begin, const char* end)
{
    MYLOG_DEBUG("Version: %.*s\n", (int)(end-begin), begin);
    return true;
}

bool CHttpEvent::on_code(const char* begin, const char* end)
{
    MYLOG_DEBUG("Code: %.*s\n", (int)(end-begin), begin);
    if (0 == strncasecmp(begin, "200", end-begin))
    {
    	_code = 200;
    }

    return true;
}

bool CHttpEvent::on_describe(const char* begin, const char* end)
{
    MYLOG_DEBUG("Describe: %.*s\n", (int)(end-begin), begin);
    return true;
}

bool CHttpEvent::on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end)
{
    MYLOG_DEBUG("[HNV] Name ==> %.*s, Value ==> %.*s\n", (int)(name_end-name_begin), name_begin, (int)(value_end-value_begin), value_begin);

    if (0 == strncasecmp(name_begin, "Content-Length", name_end-name_begin))
    {
        if (!util::CStringUtil::string2uint32(value_begin, _content_length, value_end-value_begin))
        {
            return false;         
        }
    }

    return true;
}

MOOON_NAMESPACE_END
