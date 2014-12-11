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
#ifndef MOOON_DISPATCHER_WEB_GETTER_HTTP_EVENT_H
#define MOOON_DISPATCHER_WEB_GETTER_HTTP_EVENT_H
#include <dispatcher/dispatcher.h>
#include <http_parser/http_parser.h>

class CHttpEventImpl: public http_parser::IHttpEvent
{
public:
    CHttpEventImpl();
    int32_t get_content_length() const { return _content_length; }
    void attach(dispatcher::ISender* sender);

private:
    virtual void reset();
    virtual bool on_head_end();
    virtual void on_error(const char* errmsg);        
    virtual bool on_code(const char* begin, const char* end);
    virtual bool on_describe(const char* begin, const char* end);
    virtual bool on_name_value_pair(const char* name_begin, const char* name_end
                                  , const char* value_begin, const char* value_end);

private:
    int32_t _content_length;
    dispatcher::ISender* _sender;
};

#endif // MOOON_DISPATCHER_WEB_GETTER_HTTP_EVENT_H
