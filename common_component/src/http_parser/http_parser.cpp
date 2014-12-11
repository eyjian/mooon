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
#include "parse_command.h"
HTTP_PARSER_NAMESPACE_BEGIN

class CHttpParser: public IHttpParser
{
public:
    CHttpParser(bool is_request);
    
private:
	virtual void reset();
    virtual bool head_finished() const;
    virtual int get_head_length() const;
    virtual IHttpEvent* get_http_event() const;
    virtual void set_http_event(IHttpEvent* event);
    virtual util::handle_result_t parse(const char* buf);

private:    
    bool _is_request;
    int _head_length; /** 包头字节数 */
    bool _head_finished;
    IHttpEvent* _event;
    CParseCommand* _current_command;
    CMethodCommand _method_command;
    CURLCommand _url_command;
    CVersionCommand _version_command;
    CNameValuePairCommand _name_value_pair_command;
    CHeadEndCommand _head_end_command;
    CCodeCommand _code_command;
    CDescribeCommand _describe_command;
};

CHttpParser::CHttpParser(bool is_request)
    :_is_request(is_request)
    ,_head_length(0)
    ,_event(NULL)
{
    reset();
}

void CHttpParser::reset()
{
    _head_length = 0;
    _head_finished = false;
    if (_event != NULL) _event->reset();        

    if (_is_request) // 解析http请求包
    {    
        _current_command = &_method_command;
    
	    _method_command.reset();
	    _method_command.set_next(&_url_command);	

	    _url_command.reset();
        _url_command.set_next(&_version_command);

	    _version_command.reset();
        _version_command.set_end_char('\r');
        _version_command.set_next(&_name_value_pair_command);
    }
    else // 解析http响应包
    {
        _current_command = &_version_command;

        _version_command.reset();
        _version_command.set_end_char(' ');
        _version_command.set_next(&_code_command);

        _code_command.reset();
        _code_command.set_next(&_describe_command);

        _describe_command.reset();
        _describe_command.set_next(&_name_value_pair_command);
    }

	_name_value_pair_command.reset();
    _name_value_pair_command.set_next(&_head_end_command);
    
	_head_end_command.reset();
	_head_end_command.set_next(NULL); // 循环终止条件
}

bool CHttpParser::head_finished() const
{
    return _head_finished;
}

int CHttpParser::get_head_length() const
{
    return _head_length;
}

IHttpEvent* CHttpParser::get_http_event() const
{
    return _event;
}

void CHttpParser::set_http_event(IHttpEvent* event)
{
    _event = event;
    _method_command.set_event(event);
    _url_command.set_event(event);
    _version_command.set_event(event);
    _code_command.set_event(event);
    _describe_command.set_event(event);
    _name_value_pair_command.set_event(event);
    _head_end_command.set_event(event);
}

util::handle_result_t CHttpParser::parse(const char* buffer)
{
    const char* tmp = buffer;

    for (;;)
    {
        int offset = 0;
        util::handle_result_t handle_result = _current_command->execute(tmp, offset);
        if (util::handle_error == handle_result)
		{
			return util::handle_error;
		}

        tmp += offset;
        _head_length += offset;
        
        if (util::handle_continue == handle_result) 
        {
            break;
        }
        else
        {
            _current_command = _current_command->get_next();
            if (NULL == _current_command)
			{
                _head_finished = true;
				return util::handle_finish;
			}
        }        
    }

    return util::handle_continue;
}

//////////////////////////////////////////////////////////////////////////

IHttpParser* create(bool is_request)
{
    return new CHttpParser(is_request);
}

void destroy(IHttpParser* parser)
{
    delete (CHttpParser *)parser;
}

HTTP_PARSER_NAMESPACE_END
