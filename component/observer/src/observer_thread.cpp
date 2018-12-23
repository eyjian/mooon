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
#include <mooon/sys/utils.h>
#include <mooon/utils/string_utils.h>
#include "observer_thread.h"
#include "observer_context.h"
OBSERVER_NAMESPACE_BEGIN

CObserverThread::CObserverThread(CObserverContext* observer_context, const char* thread_name_prefix)
	:_observer_context(observer_context)
{
    if (thread_name_prefix != NULL)
        _thread_name_prefix = thread_name_prefix;
}

void CObserverThread::run()
{
#if ENABLE_SET_OBSERVER_THREAD_NAME==1
    std::string thread_name;
    if (_thread_name_prefix.empty())
        thread_name = "obthread";
    else
        thread_name = _thread_name_prefix + std::string("-obthread");
    sys::CUtils::set_process_name(thread_name.c_str());
#endif // ENABLE_SET_OBSERVER_THREAD_NAME

    const uint16_t report_frequency_seconds = _observer_context->get_report_frequency_seconds();
    const int milliseconds = report_frequency_seconds * 1000;
	while (!is_stop())
	{
	    if (!_observer_context->timed_poll(milliseconds))
	        do_millisleep(milliseconds);
		_observer_context->collect();
	}

	OBSERVER_LOG_INFO("observer thread exit\n");
}

OBSERVER_NAMESPACE_END
