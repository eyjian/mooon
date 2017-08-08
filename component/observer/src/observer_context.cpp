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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#include "observer_context.h"
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/utils.h>
OBSERVER_NAMESPACE_BEGIN

CObserverContext::CObserverContext(IDataReporter* data_reporter, uint16_t report_frequency_seconds, const char* thread_name_prefix)
	:_data_reporter(data_reporter)
	,_report_frequency_seconds(report_frequency_seconds)
{
	_observer_thread = new CObserverThread(this, thread_name_prefix);
    _observer_thread->inc_refcount();
}

bool CObserverContext::create()
{
	try
	{
		_observer_thread->start();
		OBSERVER_LOG_INFO("observer thread started\n");
		return true;
	}
	catch (sys::CSyscallException& syscall_ex)
	{
		OBSERVER_LOG_FATAL("Created observer manager failed: %s.\n", syscall_ex.str().c_str());
        _observer_thread->dec_refcount();
		return false;
	}
}

void CObserverContext::destroy()
{
    OBSERVER_LOG_INFO("to stop observer thread\n");
	_observer_thread->stop();
	_observer_thread->dec_refcount();
}

void CObserverContext::register_observee(IObservable* observee)
{
    sys::LockHelper<sys::CLock> lock_helper(_lock);
	_observee_set.insert(observee);
}

void CObserverContext::deregister_objservee(IObservable* observee)
{
    sys::LockHelper<sys::CLock> lock_helper(_lock);
    _observee_set.erase(observee);
}

void CObserverContext::collect()
{
    const std::string& current_datetime = sys::get_formatted_current_datetime(true);
    sys::LockHelper<sys::CLock> lock_helper(_lock);

	for (std::set<IObservable*>::iterator iter=_observee_set.begin(); iter!=_observee_set.end(); ++iter)
	{
		IObservable* observee = *iter;
		observee->on_report(_data_reporter, current_datetime);
	}
}

//////////////////////////////////////////////////////////////////////////
// 全局函数
sys::ILogger* observer_logger = sys::g_logger;
static CObserverContext* g_observer_context = NULL;

void destroy()
{
    if (g_observer_context != NULL)
    {
        g_observer_context->destroy();
        delete g_observer_context;
        g_observer_context = NULL;
    }
}

void reset()
{
    delete g_observer_context;
    g_observer_context = NULL;
}

IObserverManager* get()
{
    return g_observer_context;
}

IObserverManager* create(IDataReporter* data_reporter, uint16_t report_frequency_seconds, const char* threadname_prefix)
{    
    if (NULL == g_observer_context) 
    {
        g_observer_context = new CObserverContext(data_reporter, report_frequency_seconds, threadname_prefix);
        if (!g_observer_context->create())
        {
            destroy();
        }
    }
    
    return g_observer_context;
}

std::string get_data_dirpath()
{
    const std::string& program_path = mooon::sys::CUtils::get_program_path();
    const std::string& data_dirpath = program_path + std::string("/../data");

    try
    {
        if (!mooon::sys::CDirUtils::exist(data_dirpath))
            mooon::sys::CDirUtils::create_directory_recursive(data_dirpath.c_str());
        return data_dirpath;
    }
    catch (mooon::sys::CSyscallException& syscall_ex)
    {
        OBSERVER_LOG_ERROR("[%s]%s\n", data_dirpath.c_str(), syscall_ex.str().c_str());
        return std::string("");
    }
}

OBSERVER_NAMESPACE_END
