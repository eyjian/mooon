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
#ifndef VIRTUAL_HOST_H
#define VIRTUAL_HOST_H
#include <string>
#include "util/log.h"
#include "util/util_config.h"
MY_NAMESPACE_BEGIN

class CVirtualHost
{
public:
    CVirtualHost(const char* domain_name);
    uint32_t get_domain_name_length() const { return _domain_name_length; }
    const char* get_domain_name() const { return _domain_name; }
    const char* const* get_ip() const { return _listen_ip; }
    uint16_t get_port() const { return _listen_port; }
    void add_ip(const char* ip);
    void set_port(uint16_t port) { _listen_port = port; }
	/** 返回文件名长度 */
    int get_full_filename(const char* short_filename, int short_filename_length, char* full_filename, int full_filename_length) const;
    void set_document_root(const char* document_root) { _document_root = document_root; }
    void set_directory_index(const char* directory_index) { directory_index = directory_index; }

private: // properties
    uint32_t _domain_name_length;
	char _domain_name[DOMAIN_NAME_MAX];
	char* _listen_ip[4]; // 一个域名最多可以绑定3个IP地址
	uint16_t _listen_port;
    std::string _document_root;
    std::string directory_index;
};

MY_NAMESPACE_END
#endif // VIRTUAL_HOST_H
