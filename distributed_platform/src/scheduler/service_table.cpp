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
#include "service_table.h"
#include "null_kernel_service.h"
SCHED_NAMESPACE_BEGIN

CServiceTable::CServiceTable()
{
	for (int i=0; i<SERVICE_ID_MAX+1; ++i)
	{
		_service_table_active[i] = new CNullKernelService();
	}
}

CServiceTable::~CServiceTable()
{
	for (int i=0; i<SERVICE_ID_MAX+1; ++i)
	{
		delete _service_table_active[i];
		_service_table_active[i] = NULL;
	}
}

bool CServiceTable::put_message(const TDistributedMessage* message)
{
	CKernelService* kernel_service = get_service(message->destination_service_id);
	return kernel_service->on_message(message);
}

int CServiceTable::load_service(const TServiceInfo& service_info)
{
	uint32_t service_id = service_info.service_id;

	if (!is_valid_service(service_id))
	{
		SCHEDULER_LOG_ERROR("Can not load illegal %s.\n", service_info.to_string().c_str());
		return E_ILLEGAL_SERVICE_ID;
	}

	// 以下操作，需要在锁的保护之下
	sys::LockHelper<sys::CLock> lock_helper(_table_lock_active[service_id]);
	sys::LockHelper<sys::CLock> lock_helper(_table_lock_new[service_id]);

	if (service_exist(service_id))
	{
		SCHEDULER_LOG_ERROR("%s exists.\n", service_info.to_string().c_str());
		return E_SERVICE_EXIST;
	}

	CKernelService* kernel_service = new CKernelService(service_info);
	int ret = kernel_service->create();
	if (E_SUCCESS ==  ret)
	{
		add_service(service_id, kernel_service);
	}
	else
	{
		delete kernel_service;
	}

	return ret;
}

int CServiceTable::unload_service(uint32_t service_id, uint32_t service_version)
{
	// 以下操作，需要在锁的保护之下
	sys::LockHelper<sys::CLock> lock_helper(_table_lock_active[service_id]);
	sys::LockHelper<sys::CLock> lock_helper(_table_lock_new[service_id]);

	bool is_active_service = true;
	CKernelService* kernel_service = _service_table_active[service_id];
	if (NULL == kernel_service)
	{
		is_active_service = false;
		kernel_service = _service_table_new[service_id];
	}
	if (NULL == kernel_service)
	{
		return E_SERVICE_NOT_EXIST;
	}

	if (service_version != kernel_service->get_service_info().service_version)
	{
		return E_VERSION_NOT_MATCH;
	}

	kernel_service->destroy();

	if (is_active_service)
	{
		kernel_service = _service_table_new[service_id];
		if (kernel_service != NULL)
		{
			// 让new成为active
			_service_table_active[service_id] = kernel_service;
			_service_table_new[service_id] = NULL;
		}
	}

	return true;
}

void CServiceTable::add_service(uint32_t service_id, CKernelService* kernel_service)
{
	if (_service_table_active[service_id] != NULL)
	{
		_service_table_active[service_id] = kernel_service;
	}
	else if (_service_table_new[service_id] != NULL)
	{
		_service_table_new[service_id] = kernel_service;
	}
	else
	{
		// 见鬼
		SCHEDULER_LOG_FATAL("Oops: %s.\n", kernel_service->to_string().c_str());
	}
}

SCHED_NAMESPACE_END
