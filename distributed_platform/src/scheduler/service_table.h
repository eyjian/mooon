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
#ifndef MOOON_SCHEDULER_SERVICE_TABLE_H
#define MOOON_SCHEDULER_SERVICE_TABLE_H
#include "kernel_service.h"
#include <sys/lock.h>
SCHED_NAMESPACE_BEGIN

class CServiceTable
{
public:
	CServiceTable();
	~CServiceTable();

	bool put_message(const TDistributedMessage* message);
	int load_service(const TServiceInfo& service_info);
	int unload_service(uint32_t service_id, uint32_t service_version);

private:
	CKernelService* get_service(uint32_t service_id)
	{
	    CKernelService* kernel_service = NULL;

	    if (is_valid_service(service_id))
	    {
	        kernel_service = _service_table_active[service_id];
            if (NULL == kernel_service)
                kernel_service = _service_table_new[service_id];
	    }

		return kernel_service;
	}

	bool service_exist(uint32_t service_id) const
	{
		return is_valid_service(service_id)
		    && _service_table_active[service_id] != NULL
		    && _service_table_new[service_id] != NULL;
	}

	void add_service(uint32_t service_id, CKernelService* kernel_service);

private:
	/***
	  * 命名说明：
	  * _active - 当前服务的Service
	  * _new - 用来替代_active版本的新版本Service
	  * 由于Service的停止需要一个过程，所以过度期，两个版本会同时处于服务状态
	  *
	  * 服务热升级过程，要求如下：
	  * 1.加载新版本Service，并让其处于服务状态
	  * 2.向主版本Service发送停止服务消息
	  * 2.新版本Service变成主版本Service
	  */
	sys::CLock _table_lock_active[SERVICE_ID_MAX + 1];
	sys::CLock _table_lock_new[SERVICE_ID_MAX + 1];
	CKernelService* _service_table_active[SERVICE_ID_MAX + 1];
	CKernelService* _service_table_new[SERVICE_ID_MAX + 1];
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_TABLE_H
