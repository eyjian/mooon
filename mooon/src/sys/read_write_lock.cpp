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

CReadWriteLock::CReadWriteLock() throw (CSyscallException)
{
	int errcode = pthread_rwlock_init(&_rwlock, NULL);
	if (errcode != 0)
	{
		pthread_rwlock_destroy(&_rwlock);
		THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_init");
	}
}

CReadWriteLock::~CReadWriteLock() throw ()
{
	pthread_rwlock_destroy(&_rwlock);
}

void CReadWriteLock::lock_read() throw (CSyscallException)
{
	int errcode = pthread_rwlock_rdlock(&_rwlock);
	if (errcode != 0)
	    THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_rdlock");
}

void CReadWriteLock::lock_write() throw (CSyscallException)
{
	int errcode = pthread_rwlock_wrlock(&_rwlock);
	if (errcode != 0)
	    THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_wrlock");
}

void CReadWriteLock::unlock() throw (CSyscallException)
{
	int errcode = pthread_rwlock_unlock(&_rwlock);
	if (errcode != 0)
	    THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_unlock");
}

bool CReadWriteLock::try_lock_read() throw (CSyscallException)
{
	int errcode = pthread_rwlock_tryrdlock(&_rwlock);

	if (0 == errcode) return true;
	if (EBUSY == errcode) return false;

	THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_tryrdlock");
}

bool CReadWriteLock::try_lock_write() throw (CSyscallException)
{
	int errcode = pthread_rwlock_trywrlock(&_rwlock);
	
	if (0 == errcode) return true;
	if (EBUSY == errcode) return false;

	THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_trywrlock");
}

bool CReadWriteLock::timed_lock_read(uint32_t millisecond) throw (CSyscallException)
{
	int errcode;

	if (0 == millisecond)
	{
	    errcode = pthread_rwlock_rdlock(&_rwlock);
	}
	else
	{	
		struct timespec abstime;

		clock_gettime(CLOCK_REALTIME, &abstime);    
		abstime.tv_sec  += millisecond / 1000;
		abstime.tv_nsec += (millisecond % 1000) * 1000000;

		errcode = pthread_rwlock_timedrdlock(&_rwlock, &abstime);
	}

	if (0 == errcode) return true;
	if (ETIMEDOUT == errcode) return false;

	THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_timedrdlock");
}

bool CReadWriteLock::timed_lock_write(uint32_t millisecond) throw (CSyscallException)
{
	int errcode;

	if (0 == millisecond)
	{
	    errcode = pthread_rwlock_trywrlock(&_rwlock);
	}
	else
	{	
		struct timespec abstime;

		clock_gettime(CLOCK_REALTIME, &abstime);    
		abstime.tv_sec  += millisecond / 1000;
		abstime.tv_nsec += (millisecond % 1000) * 1000000;

		errcode = pthread_rwlock_timedwrlock(&_rwlock, &abstime);
	}

	if (0 == errcode) return true;
	if (ETIMEDOUT == errcode) return false;

	THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_rwlock_timedwrlock");
}

SYS_NAMESPACE_END
