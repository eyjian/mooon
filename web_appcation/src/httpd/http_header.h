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
#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H
#include "util/log.h"
MY_NAMESPACE_BEGIN

class CHttpHeader
{
public:
    typedef enum
    {
        hm_unknown, hm_get, hm_head, hm_post
    }THttpMethod;

public:
    CHttpHeader();	

	void reset();
	bool get_keep_alive() const { return _keep_alive; }
	void set_keep_alive(bool keep_alive) { _keep_alive = keep_alive; }
    THttpMethod get_method() const { return _method; }
    void set_method(THttpMethod method) { _method = method; }
    const char* get_domain_name(uint16_t& length) const;
    void set_domain_name(const char* begin, uint16_t length);
    const char* get_url(uint16_t& length) const;
    void set_url(const char* begin, uint16_t length);
    
private:
	bool _keep_alive;
    THttpMethod _method;	
    uint16_t _domain_name_length; char* _domain_name;
    uint16_t _url_length; char* _url;
};

MY_NAMESPACE_END
#endif // HTTP_HEADER_H
