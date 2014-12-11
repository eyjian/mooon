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
#include "host_manager.h"
#include "http_requester.h"
#include "http_responsor.h"
#include "http_translator.h"
MY_NAMESPACE_BEGIN

bool CHttpTranslator::translate(IProtocolParser* protocol_parser, IResponsor* request_responsor)
{
    CHttpResponsor* responsor = (CHttpResponsor *)request_responsor;
    CHttpRequester* requster = (CHttpRequester *)protocol_parser;

    const CHttpHeader* header = requster->get_http_event()->get_http_header();
    uint16_t domain_name_length;
    const char* domain_name = header->get_domain_name(domain_name_length);
    
    const CVirtualHost*  host = CHostManager::get_singleton()->find_host(domain_name, domain_name_length);
    if (NULL == host)
    {
        MYLOG_ERROR("Can not find host for %.*s.\n", domain_name_length, domain_name);
        return false;
    }
    
    uint16_t url_length;
    char full_filename[FILENAME_MAX];
    const char* url = header->get_url(url_length);
    
	int full_filename_length = host->get_full_filename(url, url_length, full_filename, sizeof(full_filename)-1);
    if (0 == full_filename_length)
    {
		MYLOG_ERROR("Can not find file for %.*s/%.*s.\n", domain_name_length, domain_name, url_length, url);
        return false;
    }
        
    MYLOG_DEBUG("GET %.*s:%s.\n", domain_name_length, domain_name, full_filename);
    responsor->set_filename(full_filename, full_filename_length);    
    return true;
}

void CHttpTranslator::timeout()
{
    
}

MY_NAMESPACE_END
