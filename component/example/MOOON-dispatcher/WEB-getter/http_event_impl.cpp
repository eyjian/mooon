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
#include <strings.h>
#include <mooon/utils/string_utils.h>
#include "http_event_impl.h"

CHttpEventImpl::CHttpEventImpl()
    :_content_length(0)
    ,_sender(NULL)
{
}

void CHttpEventImpl::attach(mooon::dispatcher::ISender* sender)
{
    _sender = sender;
}

void CHttpEventImpl::reset()
{
    _content_length = 0;
}

bool CHttpEventImpl::on_head_end()
{
    return true;
}

void CHttpEventImpl::on_error(const char* errmsg)
{
    fprintf(stderr, "Response format from %s error: %s.\n"
          , _sender->str().c_str(), errmsg);
}

bool CHttpEventImpl::on_code(const char* begin, const char* end)
{
    return true;
}

bool CHttpEventImpl::on_describe(const char* begin, const char* end)
{
    return true;
}

bool CHttpEventImpl::on_name_value_pair(const char* name_begin, const char* name_end
                                      , const char* value_begin, const char* value_end)
{
    if (0 == strncasecmp(name_begin, "Content-Length", name_end-name_begin))
    {
        if (_content_length != 0)
        {
            // 已经存在，再次出现，导致了二义性，报错
            fprintf(stderr, "More than one Content-Length found from %s: %*.s.\n"
                  , _sender->str().c_str()
                  , static_cast<int>(value_end-name_begin), name_begin);
            return false;
        }
        if (!mooon::utils::CStringUtils::string2int(value_begin, _content_length, value_end-value_begin))
        {
            fprintf(stderr, "Invalid Content-Length found from %s: %*.s.\n"
                  , _sender->str().c_str()
                  , static_cast<int>(value_end-name_begin), name_begin);
            return false;
        }
        if (0 == _content_length)
        {
            fprintf(stdout, "Content-Length is 0 from %s.\n", _sender->str().c_str());
        }
        else if (_content_length < 0)
        {
            fprintf(stderr, "Invalid Content-Length found from %s: %d.\n"
                  , _sender->str().c_str(), _content_length);
            return false;
        }
        else
        {
            fprintf(stdout, "Content-Length is %d from %s.\n"
                  , _content_length, _sender->str().c_str());
        }
    }

    return true;
}
