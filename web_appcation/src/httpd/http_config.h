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
#ifndef HTTP_CONFIG_H
#define HTTP_CONFIG_H
#include "util/log.h"
#include "gtf/gtf_config.h"
#include "util/config_file.h"
MY_NAMESPACE_BEGIN

class CHttpConfig: public IGtfConfig
{
public:
	CHttpConfig(util::IConfigFile* config_file);
	~CHttpConfig();
	bool load();

private: // override
	virtual uint16_t get_thread_count() const;
	virtual uint16_t get_epoll_timeout() const;
	virtual uint32_t get_epoll_size() const;
	virtual uint32_t get_keep_alive_second() const;
	virtual uint32_t get_waiter_pool_size() const;
	virtual const TListenParameter& get_listen_parameter() const;

private:
	void do_destroy();
	bool get_thread_config();
	bool get_listen_config();
	bool build_default_host();
	bool build_virtual_host();

private:
	TListenParameter _listen_paramer;
	util::IConfigFile* _config_file;
	util::IConfigReader* _config_reader;		

private: // thread config
	uint32_t _epoll_size;
	uint32_t _waiter_number;
	uint16_t _thread_number;
	uint16_t _thread_timeout;	
	uint32_t _keep_alive_second;
};

MY_NAMESPACE_END
#endif // HTTP_CONFIG_H
