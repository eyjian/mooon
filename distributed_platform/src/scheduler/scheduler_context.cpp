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
#include "scheduler_context.h"
#include <sstream>
SCHED_NAMESPACE_BEGIN

IScheduler* create(dispatcher::IDispatcher* dispatcher)
{
	CSchedulerContext* sched_context = new CSchedulerContext(dispatcher);
	if (!sched_context->create())
	{
	    delete sched_context;
	    sched_context = NULL;
	}

	return sched_context;
}

void destroy(IScheduler* scheduler)
{
	CSchedulerContext* sched_context = static_cast<CSchedulerContext*>(scheduler);
	sched_context->destroy();
	delete sched_context;
}

////////////////////////////////////////////////////////////////////////////////
std::string TServiceInfo::to_string() const
{
	std::stringstream sstream;
	sstream << "service_info://"
			<< service_id << "/"
			<< service_version << "/"
			<< num_threads << "/"
			<< std::boolalpha << is_process_mode << "/";

	return sstream.str();
}

std::string TDistributedMessage::to_string() const
{
	std::stringstream sstream;
	sstream << "distributed_message://"
			<< header.size.to_int() << "/"
			<< header.command.to_int() << "/"
			<<< flags.to_int();

	return sstream.str();
}

////////////////////////////////////////////////////////////////////////////////
CSchedulerContext::CSchedulerContext(dispatcher::IDispatcher* dispatcher)
 :_dispatcher(dispatcher)
{
}

bool CSchedulerContext::create()
{
    return true;
}

void CSchedulerContext::destroy()
{

}

int CSchedulerContext::submit_message(const net::common_message_header* message)
{
	const TDistributedMessage* distributed_message = reinterpret_cast<const TDistributedMessage*>(message);
	return _service_table.put_message(distributed_message);
}

int CSchedulerContext::load_service(const TServiceInfo& service_info)
{
	return _service_table.load_service(service_info);
}

int CSchedulerContext::unload_service(uint32_t service_id, uint32_t service_version)
{
	return _service_table.unload_service(service_id, service_version);
}

SCHED_NAMESPACE_END
