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
#include <strings.h>
#include "host_manager.h"
#include "util/string_util.h"
MY_NAMESPACE_BEGIN

SINGLETON_IMPLEMENT(CHostManager)

CHostManager::CHostManager()
{
    for (size_t i=0; i<sizeof(_host_table)/sizeof(CVirtualHost*); ++i)
        _host_table[i] = NULL;
}

CVirtualHost* CHostManager::add_host(const char* domain_name, uint32_t domain_name_length)
{
    int times = 0;
    uint32_t factor = util::CStringUtil::hash(domain_name, domain_name_length);
	CVirtualHost* host = new CVirtualHost(domain_name);

    for (;;)
    {
        uint32_t index = factor % sizeof(_host_table)/sizeof(CVirtualHost*);
        if (NULL == _host_table[index])
        {
            _host_table[index] = host;
			MYLOG_INFO("Host: %.*s added.\n", domain_name_length, domain_name);
            return host;
        }

        ++factor;
        if (++times == sizeof(_host_table)/sizeof(CVirtualHost*)-1)
            break;
    }

	return NULL;
}

const CVirtualHost* CHostManager::find_host(const char* domain_name, uint32_t domain_name_length) const
{
    if (NULL == domain_name) return NULL;
    uint32_t factor = util::CStringUtil::hash(domain_name, domain_name_length);
    
    int times = 0;
    for (;;)
    {            
        uint32_t index = factor % sizeof(_host_table)/sizeof(CVirtualHost*);
        if (NULL == _host_table[index]) return NULL;
        
        if ((_host_table[index]->get_domain_name_length() == domain_name_length) 
         && (0 == strncasecmp(_host_table[index]->get_domain_name(), domain_name, domain_name_length)))
            return _host_table[index];

        ++factor;
        if (++times == sizeof(_host_table)/sizeof(CVirtualHost*)-1)
            return NULL;
    }
}

void CHostManager::export_listen_parameter(TListenParameter& listen_parameter) const
{
    for (size_t i=0; i<sizeof(_host_table)/sizeof(CVirtualHost*); ++i)
    {
        if (NULL == _host_table[i]) continue;

        int j;
        const char* const* ip = _host_table[i]->get_ip();
        for (j=0; ip[j][0]!='\0'; ++j)
        {
            listen_parameter.push_back(std::pair<std::string, uint16_t>(ip[j], _host_table[i]->get_port()));
        }
    }
}

MY_NAMESPACE_END
