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
#include <signal.h>
#include <mooon/sys/utils.h>
#include "context.h"
SERVER_NAMESPACE_BEGIN

// 模块日志器
sys::ILogger* logger = NULL;

////////////////////////////////////////////////////////////////////////////////
void destroy(void* server)
{
    CContext* context = static_cast<CContext*>(server);
    context->destroy();
    delete context;
}

void* create(IConfig* config, IFactory* factory)
{
    CContext* context = new CContext(config, factory);        
    if (!context->create())
    {
        delete context;
        context = NULL;
    }
    
    return context;
}

////////////////////////////////////////////////////////////////////////////////
// CServerContext

CContext::~CContext()
{
    destroy();
}

CContext::CContext(IConfig* config, IFactory* factory)
    :_config(config)
    ,_factory(factory)
{
}

bool CContext::create()
{       
    try
    {
        // 忽略PIPE信号
        if (!IgnorePipeSignal())
        {
            return false;
        }
        if (!create_listen_manager())
        {
            return false;
        }
        if (!create_thread_pool(&_listen_manager))
        {
            return false;
        }
    }
    catch (sys::CSyscallException& ex)
    {
		SERVER_LOG_FATAL("Created context failed: %s.\n", ex.str().c_str());
        return false;
    }

    return true;
}

void CContext::destroy()
{
    _thread_pool.destroy();
    _listen_manager.destroy();
}

CWorkThread* CContext::get_thread(uint16_t thread_index)
{
    return _thread_pool.get_thread(thread_index);
}

CWorkThread* CContext::get_thread(uint16_t thread_index) const
{
    return _thread_pool.get_thread(thread_index);
}

bool CContext::IgnorePipeSignal()
{
    // 忽略PIPE信号
    if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
    {
        SERVER_LOG_FATAL("Can not ignore PIPE signal for %s.\n", strerror(errno));
        return false;
    }
    else
    {
        SERVER_LOG_INFO("Ignore PIPE signal success.\n");
        return true;
    }
}

bool CContext::create_listen_manager()
{
	SERVER_LOG_INFO("Started to create listen manager.\n");    
    const net::ip_port_pair_array_t& listen_parameter = _config->get_listen_parameter();
    
	if (0 == listen_parameter.size())
    {
        SERVER_LOG_ERROR("Listen parameters are not specified.\n");
        return false;
    }
		
    for (net::ip_port_pair_array_t::size_type i=0; i<listen_parameter.size(); ++i)
    {
        _listen_manager.add(listen_parameter[i].first, listen_parameter[i].second);
		SERVER_LOG_INFO("Added listener %s:%d.\n"
		               , listen_parameter[i].first.to_string().c_str()
		               , listen_parameter[i].second);
    }

	_listen_manager.create(true);
	SERVER_LOG_INFO("Created listen manager success.\n");
    
    return true;
}

bool CContext::create_thread_pool(net::CListenManager<CListener>* listen_manager)
{
	SERVER_LOG_INFO("Started to create waiter thread pool.\n");
    if (_config->get_thread_number() < 1)
    {
        SERVER_LOG_ERROR("Server thread number is not specified.\n");
        return false;
    }

	// 创建线程池
	_thread_pool.create(_config->get_thread_number(), this);	

	uint16_t thread_count = _thread_pool.get_thread_count();
	CWorkThread** thread_array = _thread_pool.get_thread_array();

	// 设置线程运行时参数
	for (uint16_t i=0; i<thread_count; ++i)
	{
		uint16_t listen_count = listen_manager->get_listener_count();
		CListener* listener_array = listen_manager->get_listener_array();
		
		thread_array[i]->add_listener_array(listener_array, listen_count);		
	}

    _thread_pool.activate();
	SERVER_LOG_INFO("Created waiter thread pool success.\n");
    return true;
}

SERVER_NAMESPACE_END
