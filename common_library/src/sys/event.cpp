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
 * Author: jian yi, eyjian@qq.com
 */
#include <time.h>
#include <sys/time.h>
#include "sys/event.h"
SYS_NAMESPACE_BEGIN

CEvent::CEvent()
{
    int retval = pthread_cond_init(&_cond, NULL);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

CEvent::~CEvent()
{
    pthread_cond_destroy(&_cond);
}

void CEvent::wait(CLock& lock)
{
    int retval = pthread_cond_wait(&_cond, &lock._mutex);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CEvent::timed_wait(CLock& lock, uint32_t millisecond)
{
	int retval;

	if (0 == millisecond)
	{
		retval = pthread_cond_wait(&_cond, &lock._mutex);
	}
	else
	{
		struct timespec abstime;

#if _POSIX_C_SOURCE >= 199309L
        if (-1 == clock_gettime(CLOCK_REALTIME, &abstime))
            throw CSyscallException(errno, __FILE__, __LINE__);

        abstime.tv_sec  += millisecond / 1000;
        abstime.tv_nsec += (millisecond % 1000) * 1000000;
#else
        struct timeval tv;
        if (-1 == gettimeofday(&tv, NULL))
            throw CSyscallException(errno, __FILE__, __LINE__);

        abstime.tv_sec = tv.tv_sec;
        abstime.tv_nsec = tv.tv_usec * 1000;
        abstime.tv_sec  += millisecond / 1000;
        abstime.tv_nsec += (millisecond % 1000) * 1000000;
#endif // _POSIX_C_SOURCE
        
        // 处理tv_nsec溢出
        if (abstime.tv_nsec >= 1000000000L)
        {
            ++abstime.tv_sec;
            abstime.tv_nsec %= 1000000000L;
        }

		retval = pthread_cond_timedwait(&_cond, &lock._mutex, &abstime);
	}

    if (0 == retval) return true;
    if (ETIMEDOUT == retval) return false;

    throw CSyscallException(retval, __FILE__, __LINE__);
}

void CEvent::signal()
{
    int retval = pthread_cond_signal(&_cond);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

void CEvent::broadcast()
{
    int retval = pthread_cond_broadcast(&_cond);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

SYS_NAMESPACE_END
