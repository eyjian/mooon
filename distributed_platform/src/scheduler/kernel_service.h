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
#ifndef MOOON_SCHEDULER_KERNEL_SERVICE_H
#define MOOON_SCHEDULER_KERNEL_SERVICE_H
#include <sys/ref_countable.h>
#include <sys/thread_pool.h>
#include "service_bridge.h"
SCHED_NAMESPACE_BEGIN

class CKernelThread;
class CKernelService: public sys::CRefCountable
{
private:
	typedef sys::CThreadPool<CKernelThread> CKernelThreadPool;

public:
	CKernelService(const TServiceInfo& service_info);
	virtual ~CKernelService();

	int create();
	bool destroy();

	std::string to_string() const;

	CKernelService* get_new_service() const
	{
		return _new_service;
	}

	void set_new_service(CKernelService* new_service)
	{
		_new_service = new_service;
	}

public: // attributes
	IServiceBridge* get_service_bridge()
	{
		return _service_bridge;
	}

	const service_info_t& get_service_info() const
	{
		return _service_info;
	}

public:
	virtual bool on_message(const TDistributedMessage* message);

private:
	CKernelThread* choose_kernel_thread(const TDistributedMessage* message) const;

private:
	uint32_t _thread_affinity; // 用于选择Service本身（不包含Session）的消息由哪个线程处理，轮询方式
	IServiceBridge* _service_bridge;
	CKernelService* _new_service; // 新版本Service
	service_info_t _service_info;
	CKernelThreadPool _kernel_threadpool;
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_KERNEL_SERVICE_H
