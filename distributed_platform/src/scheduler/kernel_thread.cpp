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
#include "kernel_thread.h"
#include "kernel_service.h"
#include "process_message_bridge.h"
#include "thread_message_bridge.h"
SCHED_NAMESPACE_BEGIN

CKernelThread::CKernelThread()
{
}

CKernelThread::~CKernelThread()
{
	delete _message_bridge;
}

void CKernelThread::put_message(const TDistributedMessage* message)
{
	if (is_response_message(message->header.command))
	{
		_response_queue.push_back(message);
	}
	else
	{
		_request_queue.push_back(message);
	}
}

void CKernelThread::run()
{
	int num_events = _epoller.timed_wait(1000);

	for (int i=0; i<num_events; ++i)
	{
		net::CEpollable* epollable = _epoller.get(i);
		CMessageQueue* message_queue = static_cast<CMessageQueue*>(epollable);

		const TDistributedMessage* message = NULL;
		if (message_queue->pop_front(message))
		{
			_message_bridge->on_message(message);
		}
	}
}

bool CKernelThread::before_start()
{
	const service_info_t& service_info = _kernel_service->get_service_info();

	if (service_info.is_process_mode)
		_message_bridge = new CProcessMessageBridge(this);
	else
		_message_bridge = new CThreadMessageBridge(this);

	_epoller.set_events(&_request_queue, EPOLLIN);
	_epoller.set_events(&_response_queue, EPOLLIN);

	return true;
}

void CKernelThread::set_parameter(void* parameter)
{
	_kernel_service = static_cast<CKernelService*>(parameter);
}

SCHED_NAMESPACE_END
