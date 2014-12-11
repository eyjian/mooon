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
#include <string.h>
#include "virtual_host.h"
MY_NAMESPACE_BEGIN

CVirtualHost::CVirtualHost(const char* domain_name)
{
	strncpy(_domain_name, domain_name, sizeof(_domain_name)-1);
	_domain_name[sizeof(_domain_name)-1] = '\0';
    _domain_name_length = strlen(_domain_name);

    _listen_port = 0;
    for (size_t i=0; i<sizeof(_listen_ip)/sizeof(char*); ++i)
    {
        _listen_ip[i] = new char[IP_ADDRESS_MAX];
        memset(_listen_ip[i], '\0', IP_ADDRESS_MAX);
    }	
}

void CVirtualHost::add_ip(const char* ip)
{
    size_t i;
    for (i=0; i<sizeof(_listen_ip)/sizeof(char*)-1; ++i)
    {
        if ('\0' == _listen_ip[i][0])
        {
            strncpy(_listen_ip[i], ip, IP_ADDRESS_MAX-1);
            MYLOG_INFO("Virtual host %s IP: %s.\n", _domain_name, ip);
            break;
        }
    }

    if (i == sizeof(_listen_ip)/sizeof(char*)-1)
    {
        MYLOG_WARN("Ignore too many ip: %s", ip);
    }
}

int CVirtualHost::get_full_filename(const char* short_filename, int short_filename_length, char* full_filename, int full_filename_length) const
{
    if (_document_root.length()+short_filename_length > (size_t)(full_filename_length-1))
    {
        MYLOG_ERROR("Url %.*s too long.\n", short_filename_length, short_filename);
        return 0;
    }

    strncpy(full_filename, _document_root.c_str(), _document_root.length());
    strncpy(full_filename+_document_root.length(), short_filename, short_filename_length);
	(full_filename+_document_root.length())[short_filename_length] = '\0';

    return _document_root.length() + short_filename_length;
}

MY_NAMESPACE_END
