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

CEvent::CEvent() throw (CSyscallException)
{
    int errcode = pthread_cond_init(&_cond, NULL);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_cond_init");
}

CEvent::~CEvent() throw ()
{
    pthread_cond_destroy(&_cond);
}

void CEvent::wait(CLock& lock) throw (CSyscallException)
{
    int errcode = pthread_cond_wait(&_cond, &lock._mutex);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_cond_wait");
}

bool CEvent::timed_wait(CLock& lock, uint32_t millisecond) throw (CSyscallException)
{
	int errcode;

	if (0 == millisecond)
	{
	    errcode = pthread_cond_wait(&_cond, &lock._mutex);
	}
	else
	{
		struct timespec abstime;

#if _POSIX_C_SOURCE >= 199309L
        if (-1 == clock_gettime(CLOCK_REALTIME, &abstime))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "clock_gettime");

        abstime.tv_sec  += millisecond / 1000;
        abstime.tv_nsec += (millisecond % 1000) * 1000000;
#else
        struct timeval tv;
        if (-1 == gettimeofday(&tv, NULL))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "gettimeofday");

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

        errcode = pthread_cond_timedwait(&_cond, &lock._mutex, &abstime);
	}

    if (0 == errcode) return true;
    if (ETIMEDOUT == errcode) return false;

    THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_cond_timedwait");
}

void CEvent::signal() throw (CSyscallException)
{
    int errcode = pthread_cond_signal(&_cond);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_cond_signal");
}

void CEvent::broadcast() throw (CSyscallException)
{
    int errcode = pthread_cond_broadcast(&_cond);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_cond_broadcast");
}

SYS_NAMESPACE_END
