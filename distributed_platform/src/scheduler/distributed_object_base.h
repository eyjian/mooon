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
#ifndef MOOON_SCHEDULER_DISTRIBUTED_OBJECT_BASE_H
#define MOOON_SCHEDULER_DISTRIBUTED_OBJECT_BASE_H
#include <scheduler/distributed_object.h>
SCHED_NAMESPACE_BEGIN

class CDistributedObjectBase: public IDistributedObject
{
public:
    CDistributedObjectBase();

private: // implement
	virtual bool on_message(const TDistributedMessage* message);

private: // interface
	virtual bool on_user_message(const TDistributedMessage* message);
    virtual bool on_request(const TDistributedMessage* message);
    virtual bool on_response(const TDistributedMessage* message);
    virtual bool on_timer(const TDistributedMessage* message);
    virtual bool on_create_session(const TDistributedMessage* message);
    virtual bool on_destroy_session(const TDistributedMessage* message);

private:
	typedef bool (*message_proc_t)(const TDistributedMessage*);
	message_proc_t _message_proc[x];
	void init_message_proc();

	bool do_request(const TDistributedMessage* message);
	bool do_response(const TDistributedMessage* message);
	bool do_timer(const TDistributedMessage* message);
	bool do_create_session(const TDistributedMessage* message);
	bool do_destroy_session(const TDistributedMessage* message);
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_DISTRIBUTED_OBJECT_BASE_H
