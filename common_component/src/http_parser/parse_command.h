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
 * Author: JianYI, eyjian@qq.com
 */
#ifndef MOOON_HTTP_PARSER_PARSE_COMMAND_H
#define MOOON_HTTP_PARSER_PARSE_COMMAND_H
#include "http_parser/http_parser.h"
HTTP_PARSER_NAMESPACE_BEGIN

typedef bool (IHttpEvent::*on_xxx)(const char* begin, const char* end);

class CParseCommand
{
public:
    CParseCommand();
    virtual ~CParseCommand();
        
    void set_event(IHttpEvent* http_event) { _http_event = http_event; }
    void set_next(CParseCommand* next_command) { _next_command = next_command; }
    CParseCommand* get_next() const { return _next_command; }

public: // override
	virtual void reset();
    virtual util::handle_result_t execute(const char* buffer, int& offset) = 0;
        
protected:
    IHttpEvent* get_http_event() const { return _http_event; }
    
private:    
    IHttpEvent* _http_event;
    CParseCommand* _next_command;    
};

// 非名字对类，如请求行
class CNonNameValuePairCommand: public CParseCommand
{
public:
    CNonNameValuePairCommand();
	virtual void reset();
	
protected:
    util::handle_result_t do_execute(const char* buffer, int& offset, char endchar, bool (IHttpEvent::*on_xxx)(const char*, const char*));

private:
    const char* _begin;
    const char* _end;
};

class CMethodCommand: public CNonNameValuePairCommand
{
private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);
};

class CURLCommand: public CNonNameValuePairCommand
{
private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);
};

class CVersionCommand: public CNonNameValuePairCommand
{
public:
    CVersionCommand()
        :_end_char('\r')
    {        
    }

    void set_end_char(char end_char)
    {
        _end_char = end_char;
    }
    
private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);

private:
    char _end_char; // 分隔字符
};

// 响应代码，如200和403等
class CCodeCommand: public CNonNameValuePairCommand
{
private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);
};

// 响应代码的描述，如OK和Forbidden等
class CDescribeCommand: public CNonNameValuePairCommand
{
private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);
};

// 名值对类，如：Connection: KeepAlive
class CNameValuePairCommand: public CParseCommand
{
public:
    CNameValuePairCommand();
	virtual void reset();

private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);
    
private:
    bool _ignore_colon; // 是否忽略冒号，名值对以冒号分隔，但只认第一个冒号，后续的都忽略
    const char* _name_begin;
    const char* _name_end;
    const char* _value_begin;
    const char* _value_end;    
};

class CHeadEndCommand: public CParseCommand
{
private:
    virtual util::handle_result_t execute(const char* buffer, int& offset);
};

HTTP_PARSER_NAMESPACE_END
#endif // MOOON_HTTP_PARSER_PARSE_COMMAND_H
