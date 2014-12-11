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
#include <sstream>
#include <sys/util.h>
#include "getter.h"
#include "reply_handler_impl.h"

CReplyHandlerImpl::CReplyHandlerImpl(CGetter* getter)
    :_getter(getter)
    ,_sender(NULL)
    ,_state(GETTER_INIT)
    ,_response_size(0)
    ,_offset(0)
    ,_fd(-1)
{
    _http_parser = http_parser::create(false);
    _http_parser->set_http_event(&_http_event_impl);

    _length = sys::CUtil::get_page_size();
    _buffer = new char[_length];    
}

CReplyHandlerImpl::~CReplyHandlerImpl()
{
    http_parser::destroy(_http_parser);
    delete []_buffer;

    if (_fd != -1)
        close(_fd);
}

void CReplyHandlerImpl::attach(dispatcher::ISender* sender)
{
    _sender = sender;
    _http_event_impl.attach(sender);  

    _fd = open(get_filename().c_str(), O_WRONLY|O_TRUNC|O_CREAT, FILE_DEFAULT_PERM);
}

void CReplyHandlerImpl::sender_closed()
{
    _getter->connect_over(_sender);
}

void CReplyHandlerImpl::sender_connect_failure()
{
    _getter->connect_over(_sender);
}

char* CReplyHandlerImpl::get_buffer()
{
    return _buffer + _offset;
}

size_t CReplyHandlerImpl::get_buffer_length() const
{
    return _length - _offset;
}

util::handle_result_t CReplyHandlerImpl::handle_reply(size_t data_size)
{   
    write_file(data_size);
    _response_size += data_size;
    
    if (0 == _http_event_impl.get_content_length())
    {        
        // 接收HTTP头部分
        util::handle_result_t handle_result;

        handle_result = _http_parser->parse(_buffer+_offset);
        if (util::handle_continue == handle_result)
        {
            _offset += data_size;
            return handle_result;
        }
        if (handle_result != util::handle_finish)
        {
            return util::handle_close;
        }
        else
        {
            _offset = 0;
            if (0 == _http_event_impl.get_content_length())
            {
                return util::handle_close;
            }
        }
    }
    
    if (_response_size == _http_parser->get_head_length() + (int)_http_event_impl.get_content_length())
    {
        // 全部接收完了
        fprintf(stdout, "HTTP_header_length=%d, HTTP_body_length=%d from %s.\n", _http_parser->get_head_length(), (int)_http_event_impl.get_content_length(), _sender->str().c_str());
        _getter->request_success(_sender);
        return util::handle_close;
    }

    return util::handle_continue;
}

std::string CReplyHandlerImpl::get_filename() const
{
    std::stringstream filename;

    // 文件格式：html_域名_key_IP_端口_网页文件名
    filename << sys::CUtil::get_program_path()
             << "/html_"
             << _getter->get_domain_name()
             << "_"
             << _sender->get_sender_info().key
             << "_"
             << _sender->get_sender_info().ip_node.ip.to_string()
             << "_"
             << _sender->get_sender_info().ip_node.port
             << "_"
             << _getter->get_filename();

    return filename.str();
}

void CReplyHandlerImpl::write_file(size_t data_size)
{
    if (_fd != -1)
    {
        write(_fd, get_buffer(), data_size);
    }
}
