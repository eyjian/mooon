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

CLock::CLock(bool recursive)
{
    int retval = 0;
    if (recursive)
    {    
#if defined(__linux) && !defined(__USE_UNIX98)        
        const pthread_mutexattr_t attr = { PTHREAD_MUTEX_RECURSIVE_NP };
#else
        retval = pthread_mutexattr_init(&_attr);
        if (retval != 0) {
            throw CSyscallException(retval, __FILE__, __LINE__);
        }
        
        pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
        if (retval != 0) {
            pthread_mutexattr_destroy(&_attr);        
            throw CSyscallException(retval, __FILE__, __LINE__);
        }
#endif    
        retval = pthread_mutex_init(&_mutex, &_attr);
    }
    else {
        retval = pthread_mutex_init(&_mutex, NULL);
    }
    
    if (retval != 0) {
        pthread_mutexattr_destroy(&_attr);    
        throw CSyscallException(retval, __FILE__, __LINE__);
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

void CLock::lock()
{
    int retval = pthread_mutex_lock(&_mutex);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

void CLock::unlock()
{
    int retval = pthread_mutex_unlock(&_mutex);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CLock::try_lock()
{
    int retval = pthread_mutex_trylock(&_mutex);

    if (0 == retval) return true;
	if (EBUSY == retval) return false;

    throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CLock::timed_lock(uint32_t millisecond)
{
	int retval;

	if (0 == millisecond)
	{
		retval = pthread_mutex_lock(&_mutex);
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
            throw CSyscallException(errno, __FILE__, __LINE__);

        abstime.tv_sec = tv.tv_sec;
        abstime.tv_nsec = tv.tv_usec * 1000;
        abstime.tv_sec  += millisecond / 1000;
        abstime.tv_nsec += (millisecond % 1000) * 1000000;
        
		retval = pthread_mutex_timedlock(&_mutex, &abstime);
	}
	
	if (0 == retval) return true;
    if (ETIMEDOUT == retval) return false;
	
    throw CSyscallException(retval, __FILE__, __LINE__);
}

SYS_NAMESPACE_END
