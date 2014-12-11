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
#ifndef MOOON_SCHEDULER_ERROR_CODE_H
#define MOOON_SCHEDULER_ERROR_CODE_H
#include <scheduler/config.h>
SCHED_NAMESPACE_BEGIN

/***
  * 错误码常量定义
  */
enum
{
	E_SUCCESS               = 0,   // 任何成功
	E_ILLEGAL_SERVICE_ID    = 1,   // 非法的ServiceId
	E_ILLEGAL_SESSION_ID    = 2,   // 非法的SessionId
	E_SERVICE_EXIST         = 3,   // Service已经存在
	E_SERVICE_NOT_EXIST     = 4,   // Service不存在
	E_CREATE_KERNEL_THREAD  = 10,  // 创建内核线程错误
	E_CREATE_SERVICE_BRIDGE = 11,  // 创建Service桥错误
	E_VERSION_NOT_MATCH     = 12   // 版本号不匹配
};

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_ERROR_CODE_H
