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
 * Author: eyjian@gmail.com, eyjian@qq.com
 *
 */
#include "kernel_service.h"
MOOON_NAMESPACE_BEGIN

CKernelService::CKernelService(IService* service)
    :_service(service)
{
}

bool CKernelService::create()
{
    try
    {
        _schedule_thread_pool.create(_service->get_thread_number(), NULL);
        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Create scheduler thread pool for %s exception: %s.\n", _service->to_string(), ex.get_errmessage().c_str());
        return false;
    }
}

void CKernelService::destroy()
{
    try
    {
        _schedule_thread_pool.destroy();
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Destroy scheduler thread pool for %s exception: %s.\n", _service->to_string(), ex.get_errmessage().c_str());
        return false;
    }
}

MOOON_NAMESPACE_END
