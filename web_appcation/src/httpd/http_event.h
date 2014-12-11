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
#ifndef HTTP_EVENT_H
#define HTTP_EVENT_H
#include "util/log.h"
#include "http_header.h"
#include "http_parser/http_parser.h"
MY_NAMESPACE_BEGIN

class CHttpEvent: public my::IHttpEvent
{        
public:
    CHttpEvent();
	void reset();
    bool get_keep_alive() const { return _header.get_keep_alive(); }
    const CHttpHeader* get_http_header() const { return &_header; }
    
private:
    virtual bool on_head_end();
    virtual void on_error(const char* errmsg);    
    virtual bool on_method(const char* begin, const char* end);
    virtual bool on_url(const char* begin, const char* end);
    virtual bool on_version(const char* begin, const char* end);
    virtual bool on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end);

private:
    typedef bool (CHttpEvent::*on_name_value_pair_xxx)(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_3(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_4(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_5(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_6(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_7(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_8(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_9(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_10(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_11(const char* name_begin, int name_len, const char* value_begin, int value_len);
    bool on_name_value_pair_12(const char* name_begin, int name_len, const char* value_begin, int value_len);

private:
    CHttpHeader _header;
    on_name_value_pair_xxx _on_name_value_pair_xxx[20];
};

MY_NAMESPACE_END
#endif // HTTP_EVENT_H
