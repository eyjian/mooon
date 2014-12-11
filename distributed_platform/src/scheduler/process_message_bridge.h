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
#ifndef MOOON_SCHEDULER_PROCESS_MESSAGE_BRIDGE_H
#define MOOON_SCHEDULER_PROCESS_MESSAGE_BRIDGE_H
#include "message_bridge.h"
SCHED_NAMESPACE_BEGIN

// 每个CKernelThread都会有一个CProcessMessageBridge，一对一
// CProcessMessageBridge的作用是将消息传递给业务进程下对应的业务线程CServiceThread
class CKernelThread;
class CProcessMessageBridge: public IMessageBridge
{
public:
    CProcessMessageBridge(CKernelThread* kernel_thread);

private:
	virtual bool on_message(const TDistributedMessage* message);

private:
	const TDistributedMessage* read_message_from_service_thread();
	void write_message_to_service_thread(const TDistributedMessage* message);
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_PROCESS_MESSAGE_BRIDGE_H
