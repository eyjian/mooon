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
#include "sys/thread.h"
SYS_NAMESPACE_BEGIN

void* thread_proc(void* thread_param)
{
    CThread* thread = (CThread *)thread_param;
    //thread->inc_refcount(); // start中已经调用，可以确保这里可以安全的使用thread指针

    thread->run();
    thread->dec_refcount();
    return NULL;
}

CThread::CThread()
    :_lock(true)
    ,_stop(false)    
    ,_current_state(state_sleeping)
    ,_stack_size(0)
{
    int retval = pthread_attr_init(&_attr);
    if (retval != 0)
    {
        throw CSyscallException(retval, __FILE__, __LINE__);
    }
}

CThread::~CThread()
{
	pthread_attr_destroy(&_attr);
}

uint32_t CThread::get_current_thread_id()
{
    return pthread_self();
}

void CThread::start(bool detach)
{
    if (!before_start()) throw CSyscallException(0, __FILE__, __LINE__);

    // 如果本过程成功，则线程体run结束后再减引用计数，
    // 否则在失败的分支减引用计数
    this->inc_refcount();

    int retval = 0;

    // 设置线程栈大小
    if (_stack_size > 0)
        retval = pthread_attr_setstacksize(&_attr, _stack_size);
    if (0 == retval)
        retval = pthread_attr_setdetachstate(&_attr, detach?PTHREAD_CREATE_DETACHED:PTHREAD_CREATE_JOINABLE);
       
    if (0 == retval)
        retval = pthread_create(&_thread , &_attr, thread_proc, this);

    if (retval != 0)
    {
        this->dec_refcount();
        throw CSyscallException(retval, __FILE__, __LINE__);
    }
}

size_t CThread::get_stack_size() const
{
    size_t stack_size = 0;
    int retval = pthread_attr_getstacksize(&_attr, &stack_size);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);

    return stack_size;
}

void CThread::join()
{
    // 线程自己不能调用join
    if (CThread::get_current_thread_id() != this->get_thread_id())
    {    
        int retval = pthread_join(_thread, NULL);
        if (retval != 0)
            throw CSyscallException(retval, __FILE__, __LINE__);
    }
}

void CThread::detach()
{
    int retval = pthread_detach(_thread);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CThread::can_join() const
{
    int detachstate;
    int retval = pthread_attr_getdetachstate(&_attr, &detachstate);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);

    return (PTHREAD_CREATE_JOINABLE == detachstate);
}

bool CThread::is_stop() const
{
    return _stop;
}

void CThread::do_wakeup(bool stop)
{   
    // 线程终止标识
    if (stop) _stop = stop;
    
    // 保证在唤醒线程之前，已经将它的状态修改为state_wakeup
    if (state_sleeping == _current_state)
    {
        _current_state = state_wakeuped;
        _event.signal();  
    }
    else
    {
        _current_state = state_wakeuped;
    }
}

void CThread::wakeup()
{
    LockHelper<CLock> lock_helper(_lock);    
    do_wakeup(false);    
}

void CThread::stop(bool wait_stop)
{
    if (!_stop)
    {
        _stop = true;
        before_stop();
        LockHelper<CLock> lock_helper(_lock);
        do_wakeup(true);            
    }
    if (wait_stop && can_join())
    {
        join();
    }
}

void CThread::do_millisleep(int milliseconds)
{
    // 非本线程调用无效
    if (this->get_thread_id() == CThread::get_current_thread_id())
    {    
        LockHelper<CLock> lock_helper(_lock);
        if (!is_stop())
        {
            if (_current_state != state_wakeuped)
            {        
                _current_state = state_sleeping;
                if (milliseconds < 0)
                    _event.wait(_lock);
                else
                    _event.timed_wait(_lock, milliseconds);                
            }

            // 不设置为state_wakeup，以保证可以再次调用do_millisleep
            _current_state = state_running;
        }
    }
}

SYS_NAMESPACE_END
