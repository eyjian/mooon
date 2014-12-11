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
 * Author: jian yi, eyjian@qq.com
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include "util/md5.h"
#include "util/log.h"
#include "sys/sys_util.h"
#include "http_responsor.h"
MY_NAMESPACE_BEGIN

CHttpResponsor::CHttpResponsor(CHttpRequester* requester)
    :_response_code(0)
    ,_requester(requester)
	,_cache_entity(NULL)
{        
    _header_buffer = new char[1024];    
	reset(); // 必须在new之后
}

CHttpResponsor::~CHttpResponsor()
{
	reset();
    delete []_header_buffer;
}

void CHttpResponsor::reset()
{    
	_fd          = -1;
	_response_code = 0;
    _header_length = 0;
    _header_offset = 0;
	_body_length   = 0;
    _body_offset   = 0;
	_header_buffer[0] = '\0';
	_body_buffer = NULL;
	_send_header = true;	
}

int CHttpResponsor::send_file(int sockfd)
{
	off_t offset = 0;
    return sendfile(sockfd, _fd, &offset, _body_length-_body_offset);
}

off_t CHttpResponsor::get_buffer_length() const
{
	// 还没有发送响应包头
    if (_send_header)
	{
		if (0 == _header_length)
        {
            if (keep_alive())
			{
			    _header_length = snprintf(_header_buffer
					,1024, "HTTP/1.1 %d OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\nConnection: Keep-Alive\r\n\r\n"
					,_response_code
					,_body_length);
			}
            else
			{
                _header_length = snprintf(_header_buffer
					,1024, "HTTP/1.1 %d OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\nConnection: Close\r\n\r\n"
					,_response_code
					,_body_length);
			}
        }
		return _header_length-_header_offset;
	}
	else
	{
		return _body_length-_body_offset;
	}
}

char* CHttpResponsor::get_buffer()
{
	// 还在发送包头
	if (_send_header) return _header_buffer+_header_offset;
	
	// 发送包体	
	return _body_buffer;
}

void CHttpResponsor::offset_buffer(off_t offset)
{
    if (_send_header)
	{
		_header_offset += offset;

		// 包头发送完毕，切换状态，以便发送包体
		if (_header_offset == _header_length)
			_send_header = false;
	}
	else
	{
		_body_offset += offset;
		if (_body_offset == _body_length)
		{
			if (_cache_entity != NULL)
			{
				CHttpCache::get_singleton()->release_cache_entity(_cache_entity);
			}
		}
	}
}

bool CHttpResponsor::keep_alive() const
{
    return _requester->get_keep_alive() && (_response_code == 200);
}

void CHttpResponsor::set_filename(const char* filename, int filename_length)
{
	_cache_entity  = CHttpCache::get_singleton()->get_cache_entity(filename, filename_length);
	_response_code = (NULL == _cache_entity)? 404: 200;
	_body_length   = (NULL == _cache_entity)? 0: _cache_entity->get_map()->len;
	_body_buffer   = (NULL == _cache_entity)? NULL: (char *)_cache_entity->get_map()->addr;
	_fd            = (NULL == _cache_entity)? -1: _cache_entity->get_map()->fd;
}

MY_NAMESPACE_END
