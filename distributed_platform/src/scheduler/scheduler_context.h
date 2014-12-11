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
#ifndef MOOON_SCHEDULER_CONTEXT_H
#define MOOON_SCHEDULER_CONTEXT_H
#include <scheduler/scheduler.h>
#include "service_table.h"
SCHED_NAMESPACE_BEGIN

class CSchedulerContext: public IScheduler
{
public:
	CSchedulerContext(dispatcher::IDispatcher* dispatcher);
	bool create();
	void destroy();

private: // override
	virtual int submit_message(const net::common_message_header* message);
	virtual int load_service(const TServiceInfo& service_info);
	virtual int unload_service(uint32_t service_id, uint32_t service_version);

private:
	CServiceTable _service_table;
	dispatcher::IDispatcher* _dispatcher;
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_CONTEXT_H
