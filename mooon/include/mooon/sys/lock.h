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
#ifndef MOOON_SYS_LOCK_H
#define MOOON_SYS_LOCK_H
#include "mooon/sys/utils.h"
#include <pthread.h>
SYS_NAMESPACE_BEGIN

/***
  * 互斥锁类
  * 对于非递归锁，同一线程在未释放上一次加的锁之前，
  * 不能连续两次去加同一把锁，但递归锁允许同一线程连续对同一把锁加多次
  */
class CLock
{
    friend class CEvent; // CEvent需要访问CLock的_mutex成员

public:
    /***
      * 构造一个互斥锁
      * @recursive: 是否构造为递归锁
      * @exception: 出错抛出CSyscallException异常
      */
    CLock(bool recursive = false) throw (CSyscallException);
    ~CLock();

    /***
      * 加锁操作，如果不能获取到锁，则一直等待到获取到锁为止
      * @exception: 出错抛出CSyscallException异常
      */
    void lock() throw (CSyscallException);

    /***
      * 解锁操作
      * 请注意必须已经调用了lock加锁，才能调用unlock解锁
      * @exception: 出错抛出CSyscallException异常
      */
    void unlock() throw (CSyscallException);

    /***
      * 尝试性的去获取锁，如果得不到锁，则立即返回
      * @return: 如果获取到了锁，则返回true，否则返回false
      * @exception: 出错抛出CSyscallException异常
      */
    bool try_lock() throw (CSyscallException);

    /***
      * 以超时方式去获取锁，如果指定的毫秒时间内不能获取到锁，则一直等待直到超时
      * @return: 如果在指定的毫秒时间内获取到了锁，则返回true，否则如果超时则返回false
      * @exception: 出错抛出CSyscallException异常
      */
	bool timed_lock(uint32_t millisecond) throw (CSyscallException);

private:
    pthread_mutex_t _mutex;
    pthread_mutexattr_t _attr;
};

/***
  *递归锁类
  * 对于非递归锁，同一线程在未释放上一次加的锁之前，
  * 不能连续两次去加同一把锁，但递归锁允许同一线程连续对同一把锁加多次
  */
class CRecLock: public CLock
{
public:
    /***
      * 构造一个递归锁
      * @exception: 出错抛出CSyscallException异常
      */
    CRecLock() throw (CSyscallException)
        :CLock(true)
    {
    }
};

/***
  * 锁帮助类，用于自动解锁
  */
template <class LockClass>
class LockHelper
{
public:
    /***
      * 构造锁帮助类对象
      * @exception: 出错抛出CSyscallException异常
      */
    LockHelper(LockClass& lock) throw (CSyscallException)
        :_lock(lock)
    {
        lock.lock();
    }

    /***
      * 析构函数，会自动调用unlock解锁
      * @exception: 析构函数不允许抛出任何异常
      */
    ~LockHelper()
    {
        try
        {
            _lock.unlock();
        }
        catch (...)
        {
        }
    }

private:
    LockClass& _lock;
};

/***
  * 空锁，实际上就是不加锁
  * 用于可以自由切换线程安全的模板类实现
  */
class CNullLock
{
public:
	void lock()
	{
	}

	void unlock()
	{
	}
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_LOCK_H
