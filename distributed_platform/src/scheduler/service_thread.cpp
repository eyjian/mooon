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
#include "service_thread.h"
#include "distributed_object_table.h"
#include "syscall_processor.h"
SCHED_NAMESPACE_BEGIN

void CServiceThread::run()
{
	const TDistributedMessage* message = read_from_process_message_bridge();
	const TDistributedMessage* distribted_message = reinterpret_cast<const distribted_message_t*>(message);

	IDistributedObject* distributed_object = _distributed_object_table->get_object(distribted_message->session_id);
	if (distributed_object != NULL)
	{
		distributed_object->on_message(distribted_message);
	}
}

const TDistributedMessage* CServiceThread::read_from_process_message_bridge()
{
    return NULL;
}

SCHED_NAMESPACE_END
