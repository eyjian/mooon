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
#ifndef MOOON_SCHEDULER_NULL_DISTRIBUTED_OBJECT_H
#define MOOON_SCHEDULER_NULL_DISTRIBUTED_OBJECT_H
#include <scheduler/distributed_object.h>
SCHED_NAMESPACE_BEGIN

class CNullDistributedObject: public IDistributedObject
{
private:
	virtual bool on_message(const TDistributedMessage* message);
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_NULL_DISTRIBUTED_OBJECT_H
