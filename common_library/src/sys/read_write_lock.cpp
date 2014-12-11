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
#include "sys/read_write_lock.h"
SYS_NAMESPACE_BEGIN

CReadWriteLock::CReadWriteLock()
{
	int retval = pthread_rwlock_init(&_rwlock, NULL);
	if (retval != 0) 
	{
		pthread_rwlock_destroy(&_rwlock);
		throw CSyscallException(retval, __FILE__, __LINE__);
	}
}

CReadWriteLock::~CReadWriteLock()
{
	pthread_rwlock_destroy(&_rwlock);
}

void CReadWriteLock::lock_read()
{
	int retval = pthread_rwlock_rdlock(&_rwlock);
	if (retval != 0)
		throw CSyscallException(retval, __FILE__, __LINE__);
}

void CReadWriteLock::lock_write()
{
	int retval = pthread_rwlock_wrlock(&_rwlock);
	if (retval != 0)
		throw CSyscallException(retval, __FILE__, __LINE__);
}

void CReadWriteLock::unlock()
{
	int retval = pthread_rwlock_unlock(&_rwlock);
	if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CReadWriteLock::try_lock_read()
{
	int retval = pthread_rwlock_tryrdlock(&_rwlock);

	if (0 == retval) return true;	
	if (EBUSY == retval) return false;

	throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CReadWriteLock::try_lock_write()
{
	int retval = pthread_rwlock_trywrlock(&_rwlock);
	
	if (0 == retval) return true;	
	if (EBUSY == retval) return false;

	throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CReadWriteLock::timed_lock_read(uint32_t millisecond)
{
	int retval;

	if (0 == millisecond)
	{
		retval = pthread_rwlock_rdlock(&_rwlock);
	}
	else
	{	
		struct timespec abstime;

		clock_gettime(CLOCK_REALTIME, &abstime);    
		abstime.tv_sec  += millisecond / 1000;
		abstime.tv_nsec += (millisecond % 1000) * 1000000;

		retval = pthread_rwlock_timedrdlock(&_rwlock, &abstime);
	}

	if (0 == retval) return true;
	if (ETIMEDOUT == retval) return false;

	throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CReadWriteLock::timed_lock_write(uint32_t millisecond)
{
	int retval;

	if (0 == millisecond)
	{
		retval = pthread_rwlock_trywrlock(&_rwlock);
	}
	else
	{	
		struct timespec abstime;

		clock_gettime(CLOCK_REALTIME, &abstime);    
		abstime.tv_sec  += millisecond / 1000;
		abstime.tv_nsec += (millisecond % 1000) * 1000000;

		retval = pthread_rwlock_timedwrlock(&_rwlock, &abstime);
	}

	if (0 == retval) return true;
	if (ETIMEDOUT == retval) return false;

	throw CSyscallException(retval, __FILE__, __LINE__);
}

SYS_NAMESPACE_END
