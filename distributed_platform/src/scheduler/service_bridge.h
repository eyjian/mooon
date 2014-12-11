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
#ifndef MOOON_SCHEDULER_SERVICE_BRIDGE_H
#define MOOON_SCHEDULER_SERVICE_BRIDGE_H
#include <scheduler/scheduler.h>
#include "scheduler_log.h"
SCHED_NAMESPACE_BEGIN

class CKernelService;
class IServiceBridge
{
public:
	IServiceBridge(CKernelService* kernel_service);
	virtual ~IServiceBridge() {}

	virtual bool create() = 0;
	virtual bool destroy() = 0;

	std::string to_string() const;
	const service_info_t& get_service_info() const;

protected:
	CKernelService* _kernel_service;
};

extern IServiceBridge* create_service_bridge(CKernelService* kernel_service);

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_BRIDGE_H
