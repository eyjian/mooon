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
#include "sys/lock.h"
SYS_NAMESPACE_BEGIN

CLock::CLock(bool recursive) throw (CSyscallException)
{
    int errcode = 0;
    if (recursive)
    {    
#if defined(__linux) && !defined(__USE_UNIX98)        
        const pthread_mutexattr_t attr = { PTHREAD_MUTEX_RECURSIVE_NP };
#else
        errcode = pthread_mutexattr_init(&_attr);
        if (errcode != 0)
            THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutexattr_init");
        
        pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
        if (errcode != 0)
        {
            pthread_mutexattr_destroy(&_attr);        
            THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutexattr_settype");
        }
#endif    
        errcode = pthread_mutex_init(&_mutex, &_attr);
    }
    else
    {
        errcode = pthread_mutex_init(&_mutex, NULL);
    }
    
    if (errcode != 0)
    {
        pthread_mutexattr_destroy(&_attr);    
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutex_init");
    }
}

CLock::~CLock()
{
#if defined(__linux) && !defined(__USE_UNIX98) 
#else
    pthread_mutexattr_destroy(&_attr);
#endif
    pthread_mutex_destroy(&_mutex);
}

void CLock::lock() throw (CSyscallException)
{
    int errcode = pthread_mutex_lock(&_mutex);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutex_lock");
}

void CLock::unlock() throw (CSyscallException)
{
    int errcode = pthread_mutex_unlock(&_mutex);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutex_unlock");
}

bool CLock::try_lock() throw (CSyscallException)
{
    int errcode = pthread_mutex_trylock(&_mutex);

    if (0 == errcode) return true;
	if (EBUSY == errcode) return false;

	THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutex_trylock");
}

bool CLock::timed_lock(uint32_t millisecond) throw (CSyscallException)
{
	int errcode;

	if (0 == millisecond)
	{
	    errcode = pthread_mutex_lock(&_mutex);
	}
	else
	{	
		struct timespec abstime;

#if _POSIX_C_SOURCE >= 199309L
		clock_gettime(CLOCK_REALTIME, &abstime);    
		abstime.tv_sec  += millisecond / 1000;
		abstime.tv_nsec += (millisecond % 1000) * 1000000;
#else
#endif // _POSIX_C_SOURCE
        struct timeval tv;
        if (-1 == gettimeofday(&tv, NULL))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "gettimeofday");

        abstime.tv_sec = tv.tv_sec;
        abstime.tv_nsec = tv.tv_usec * 1000;
        abstime.tv_sec  += millisecond / 1000;
        abstime.tv_nsec += (millisecond % 1000) * 1000000;
        
        errcode = pthread_mutex_timedlock(&_mutex, &abstime);
	}
	
	if (0 == errcode) return true;
    if (ETIMEDOUT == errcode) return false;
	
    THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_mutex_timedlock");
}

SYS_NAMESPACE_END
