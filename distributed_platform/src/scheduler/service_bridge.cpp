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
#include "process_bridge.h"
#include "thread_bridge.h"
#include "kernel_service.h"
SCHED_NAMESPACE_BEGIN

IServiceBridge* create_service_bridge(CKernelService* kernel_service)
{
	bool is_process_mode = kernel_service->get_service_info().is_process_mode;

	return is_process_mode
		 ? new CProcessBridge(kernel_service)
	     : new CThreadBridge(kernel_service);
}

IServiceBridge::IServiceBridge(CKernelService* kernel_service)
 :_kernel_service(kernel_service)
{
}

std::string IServiceBridge::to_string() const
{
	return _kernel_service->to_string();
}

const service_info_t& IServiceBridge::get_service_info() const
{
	return _kernel_service->get_service_info();
}

SCHED_NAMESPACE_END
