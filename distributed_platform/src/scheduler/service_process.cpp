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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include "service_process.h"
#include <sys/shared_library.h>
#include <sys/thread_pool.h>
#include <sys/util.h>
#include "process_bridge.h"
#include "service_thread.h"
SCHED_NAMESPACE_BEGIN

CServiceProcess::CServiceProcess(CProcessBridge* process_bridge)
 :_process_bridge(process_bridge)
{
}

bool CServiceProcess::create()
{
	const service_info_t& service_info = _process_bridge->get_service_info();

	if (-1 == pipe(_pipe_fd))
	{
		SCHEDULER_LOG_ERROR("%s pipe error: %s.\n"
				          , _process_bridge->to_string().c_str()
				          , sys::CUtil::get_last_error_message().c_str());
		return false;
	}

	_service_pid = fork();
	if (-1 == _service_pid)
	{
		SCHEDULER_LOG_ERROR("%s fork error: %s.\n"
				           , _process_bridge->to_string().c_str()
				           , sys::CUtil::get_last_error_message().c_str());

		close(_pipe_fd[0]);
		close(_pipe_fd[1]);
		return false;
	}
	if (0 == _service_pid)
	{
		close(_pipe_fd[0]);
		typedef sys::CThreadPool<CServiceThread> CServiceThreadPool;

		CServiceThreadPool service_threadpool;

		try
		{
			service_threadpool.create(service_info.num_threads, _process_bridge);

			close(_pipe_fd[1]); // 成功，不用写东西
			exit(0); // 别忘记了，退出Service子进程
		}
		catch (sys::CSyscallException& ex)
		{
			sys::CUtil::common_pipe_write(fd[1], ex.to_string().c_str(), ex.to_string().size());
			exit(1);
		}
	}

	SCHEDULER_LOG_INFO("%s PID is %u.\n", _process_bridge->to_string().c_str(), _service_pid);
	close(_pipe_fd[1]);

	char* str = NULL;
	uint32_t str_size = 0;

	try
	{
		sys::CUtil::common_pipe_read(_pipe_fd[0], &str, &str_size);
		if (0 == str_size)
		{
			SCHEDULER_LOG_INFO("%s PID[%u] start successfully.\n", _process_bridge->to_string().c_str(), _service_pid);
		}
		else
		{
			util::DeleteHelper<char> dh(str, true);
			SCHEDULER_LOG_INFO("%s PID[%u] start failed: %s.\n", _process_bridge->to_string().c_str(), _service_pid, str);
		}
	}
	catch (sys::CSyscallException& ex)
	{
		SCHEDULER_LOG_ERROR("%s PID is %u.\n", _process_bridge->to_string().c_str(), _service_pid);
		return false;
	}

	return true;
}

bool CServiceProcess::destroy()
{
	return true;
}

SCHED_NAMESPACE_END
