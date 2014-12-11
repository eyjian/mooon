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
#include "kernel_service.h"
#include "kernel_thread.h"
#include "process_bridge.h"
#include "thread_bridge.h"
SCHED_NAMESPACE_BEGIN

CKernelService::CKernelService(const TServiceInfo& service_info)
 :_thread_affinity(0)
 ,_service_bridge(NULL)
 ,_new_service(NULL)
 ,_service_info(service_info)
{
}

CKernelService::~CKernelService()
{
	delete _service_bridge;
}

int CKernelService::create()
{
	try
	{
		_kernel_threadpool.create(_service_info.num_threads, this);
	}
	catch (sys::CSyscallException& ex)
	{
		SCHEDULER_LOG_ERROR("%s: %s.\n", to_string().c_str(), ex.to_string().c_str());
		return E_CREATE_KERNEL_THREAD;
	}

	_service_bridge = create_service_bridge(this);

	try
	{
		_service_bridge->create();
	}
	catch (sys::CSyscallException& ex)
	{
		SCHEDULER_LOG_ERROR("%s: %s.\n", to_string().c_str(), ex.to_string().c_str());

		_kernel_threadpool.destroy();
		delete _service_bridge;
		_service_bridge = NULL;

		return E_CREATE_SERVICE_BRIDGE;
	}

	return E_SUCCESS;
}

bool CKernelService::destroy()
{
	_service_bridge->destroy();
	_kernel_threadpool.destroy();
	return true;
}

std::string CKernelService::to_string() const
{
	return _service_info.to_string();
}

bool CKernelService::on_message(const TDistributedMessage* message)
{
	CKernelThread* kernel_thread = choose_kernel_thread(message);
	return kernel_thread->put_message(message);
}

CKernelThread* CKernelService::choose_kernel_thread(const TDistributedMessage* message) const
{
	uint16_t thread_index = 0;
	const net::TCommonMessageHeader* message_header = &message->header;

	if (is_service_message(message_header->command))
	{
		// 轮询方式分配消息
		thread_index = ++_thread_affinity;
	}
	else if (is_session_message(message_header->command))
	{
		// 仅由线程亲和值thread_affinity决定
		thread_index = message->thread_affinity % _kernel_threadpool.get_thread_count();
	}
	else
	{
		SCHEDULER_LOG_ERROR("Invalid %s.\n", message->to_string().c_str());
		return NULL; // Invalid message
	}

	thread_index = thread_index % _kernel_threadpool.get_thread_count();
	return _kernel_threadpool.get_thread(thread_index);
}

SCHED_NAMESPACE_END
