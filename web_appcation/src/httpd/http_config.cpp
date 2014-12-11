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
#include "http_config.h"
#include "host_manager.h"
#include "net/net_util.h"
#include "util/token_list.h"
#include "util/string_util.h"
#include "plugin/plugin_tinyxml/plugin_tinyxml.h"
MY_NAMESPACE_BEGIN

/*
<?xml version="1.0" encoding="gb2312"?>
<JWS>
	<thread number="1" timeout="1000" waiter_number="1000" epoll_size="10000" />
	
	<listen ip="eth1" port="8000" />
	<document root="/" index="index.htm" />
		
	<virtual_host domain_name="www.hadoopor.com">
		<listen ip="eth1" port="8000" />
		<document root="/" index="index.htm" />
	</virtual_host>
</JWS>
*/

CHttpConfig::CHttpConfig(util::IConfigFile* config_file)
	:_config_file(config_file)
    ,_config_reader(NULL)
{
	_epoll_size = 0;
	_waiter_number = 0;
	_thread_number = 0;
	_thread_timeout = 0;
    _keep_alive_second = 0;
}

CHttpConfig::~CHttpConfig()
{
	do_destroy();
}

void CHttpConfig::do_destroy()
{
	if (_config_file != NULL)
	{
        if (_config_reader != NULL)
            _config_file->release_config_reader(_config_reader);

		plugin::destroy_config_file(_config_file);
		_config_file = NULL;
	}
}

uint16_t CHttpConfig::get_thread_count() const
{
    return _thread_number;
}

uint16_t CHttpConfig::get_epoll_timeout() const
{
    return _thread_timeout;
}

uint32_t CHttpConfig::get_waiter_pool_size() const
{
    return _waiter_number;
}

uint32_t CHttpConfig::get_epoll_size() const
{
	return _epoll_size;
}

uint32_t CHttpConfig::get_keep_alive_second() const
{
    return _keep_alive_second;
}

const TListenParameter& CHttpConfig::get_listen_parameter() const
{    	
    return _listen_paramer;
}

bool CHttpConfig::load()
{
	for (;;)
	{	
        _config_reader = _config_file->get_config_reader();
        if (NULL == _config_reader)
		{
			MYLOG_ERROR("Can not get config reader.\n");
			break;
		}
        
		if (!get_thread_config()) break;
		if (!get_listen_config()) break;
	
		return true;
	}

	do_destroy();
	return false;
}

bool CHttpConfig::get_thread_config()
{
	// thread number
	if (!_config_reader->get_uint16_value("/JWS/thread", "number", _thread_number))
	{
		MYLOG_ERROR("Can not get thread number from \"/JWS/thread:number\".\n");
		return false;
	}
	MYLOG_INFO("\"/JWS/thread:number\" is %d.\n", _thread_number);

	// epoll timeout
	if (!_config_reader->get_uint16_value("/JWS/thread", "timeout", _thread_timeout))
	{
		MYLOG_ERROR("Can not get thread timeout from \"/JWS/thread:timeout\".\n");
		return false;
	}
	MYLOG_INFO("\"/JWS/thread:timeout\" is %d.\n", _thread_timeout);
	
	// waiter number
	if (!_config_reader->get_uint32_value("/JWS/thread", "waiter_number", _waiter_number))
	{
		MYLOG_ERROR("Can not get waiter number from \"/JWS/thread:waiter_number\".\n");
		return false;
	}
	MYLOG_INFO("\"/JWS/thread:waiter_number\" is %u.\n", _waiter_number);
	
	// epoll size
	if (!_config_reader->get_uint32_value("/JWS/thread", "epoll_size", _epoll_size))
	{
		MYLOG_ERROR("Can not get epoll size from \"/JWS/thread:epoll_size\".\n");
		return false;
	}
	MYLOG_INFO("\"/JWS/thread:epoll_size\" is %u.\n", _epoll_size);

    // Keep alive    
    if (!_config_reader->get_uint32_value("/JWS/thread", "keep_alive_second", _keep_alive_second))
	{
		MYLOG_ERROR("Can not get epoll size from \"/JWS/thread:keep_alive_second\".\n");
		return false;
	}
	MYLOG_INFO("\"/JWS/thread:keep_alive_second\" is %u.\n", _keep_alive_second);

	return true;
}

bool CHttpConfig::get_listen_config()
{
	return build_default_host() && build_virtual_host();
}

bool CHttpConfig::build_default_host()
{
	std::string ip;
	if (!_config_reader->get_string_value("/JWS/listen", "ip", ip))
	{
		MYLOG_WARN("Not configured default host at \"/JWS/listen:ip\".\n");
		return true;
	}
	MYLOG_INFO("\"/JWS/listen:ip\" is %s.\n", ip.c_str());

	uint16_t port = 0;
	if (!_config_reader->get_uint16_value("/JWS/listen", "port", port))
	{
		MYLOG_WARN("Default listen port not configured at \"/JWS/listen:port\".\n");
		return true;
	}
	MYLOG_INFO("\"/JWS/listen:port\" is %d.\n", port);

	return true;
}

bool CHttpConfig::build_virtual_host()
{
	std::vector<util::IConfigReader*> sub_config_array;
	if (!_config_reader->get_sub_config("/JWS/virtual_host", sub_config_array))
	{
		MYLOG_WARN("Not found virtual host at \"/JWS/virtual_host\".\n");
		return true;
	}

	for (std::vector<util::IConfigReader*>::size_type i=0; i<sub_config_array.size(); ++i)
	{
        uint16_t port;
        std::string ip;
        std::string domain_name;
        std::string document_root;
        std::string directory_index;              
        std::string domain_name_array;
        CVirtualHost* host = NULL;

        if (!sub_config_array[i]->get_string_value("/virtual_host", "domain_name", domain_name_array))
        {
	        MYLOG_ERROR("Domain name not found for virtual host at \"/virtual_host:domain_name\".\n");
	        return false;
        }
        else
        {
	        MYLOG_INFO("Virtual host domain names: %s.\n", domain_name_array.c_str());            
        }

        util::CTokenList::TTokenList token_list;
        util::CTokenList::parse(token_list, domain_name_array, ",");

        if (0 == token_list.size())
        {
            MYLOG_ERROR("Domain name error for virtual host at \"/virtual_host:domain_name\".\n");
            return false;
        }

        for (util::CTokenList::TTokenList::iterator iter=token_list.begin(); iter!=token_list.end(); ++iter)
        {
            domain_name = *iter;
            
            if (!sub_config_array[i]->get_uint16_value("/virtual_host/listen", "port", port))
            {
	            // 使用默认的
	            MYLOG_WARN("Can not get port for virtual host %s.\n", domain_name.c_str());
            }
            else
            {
	            MYLOG_INFO("Virtual host %s port: %d.\n", domain_name.c_str(), port);            
            }

            if (port != 80)
                domain_name += std::string(":") + util::CStringUtil::uint16tostring(port);

            host = CHostManager::get_singleton()->add_host(domain_name.c_str(), domain_name.length());
            host->set_port(port);

            if (!sub_config_array[i]->get_string_value("/virtual_host/listen", "ip", ip))
            {
	            // 使用默认的
	            MYLOG_WARN("Can not get IP for virtual host %s.\n", domain_name.c_str());
            }
            else
            {
	            MYLOG_INFO("Virtual host %s IP: %s.\n", domain_name.c_str(), ip.c_str());
                if (0 == strncmp(ip.c_str(), "eth", 3))
                {
                    net::CNetUtil::TIPArray ip_array;
                    net::CNetUtil::get_ethx_ip(ip.c_str(), ip_array);
                    if (0 == ip_array.size())
                    {
                        MYLOG_ERROR("Error eth name: %s at \"/virtual_host/listen:ip\".\n", ip.c_str());
                        return false;
                    }

                    for (net::CNetUtil::TIPArray::size_type i=0; i<ip_array.size(); ++i)
                    {
                        host->add_ip(ip_array[i].c_str());
                    }
                }
                else
                {
                    util::CTokenList::TTokenList token_list;
                    util::CTokenList::parse(token_list, ip, ",");
                    util::CTokenList::TTokenList::iterator iter = token_list.begin();
                    while (iter++ != token_list.end())
                    {
                        host->add_ip(iter->c_str());
                    }
                }
            }        

            // Document root
            if (!sub_config_array[i]->get_string_value("/virtual_host/document", "root", document_root))
            {
                MYLOG_WARN("Can not get document root for virtual host %s.\n", domain_name.c_str());
            }
            else
            {
                MYLOG_INFO("Virtual host %s document root: %s.\n", domain_name.c_str(), document_root.c_str());
                host->set_document_root(document_root.c_str());
            }

            // Directory index
            if (!sub_config_array[i]->get_string_value("/virtual_host/document", "index", directory_index))
            {
                MYLOG_WARN("Can not get directory index for virtual host %s.\n", domain_name.c_str());
            }
            else
            {
                MYLOG_INFO("Virtual host %s directory index: %s.\n", domain_name.c_str(), directory_index.c_str());
                host->set_directory_index(directory_index.c_str());
            }
        }

        _config_file->release_config_reader(sub_config_array[i]);		
	}
    
	sub_config_array.clear();    
    CHostManager::get_singleton()->export_listen_parameter(_listen_paramer);

    return true;
}

MY_NAMESPACE_END
