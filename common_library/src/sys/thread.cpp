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

static void* thread_proc(void* thread_param)
{
    CThread* thread = (CThread *)thread_param;
    //thread->inc_refcount(); // start中已经调用，可以确保这里可以安全的使用thread指针

    thread->run();
    thread->dec_refcount();
    return NULL;
}

CThread::CThread() throw (utils::CException, CSyscallException)
    :_lock(true)
    ,_stop(false)
    ,_current_state(state_sleeping)
    ,_thread(0)
    ,_stack_size(0)
{
    int errcode = pthread_attr_init(&_attr);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_attr_init");
}

CThread::~CThread()
{
	pthread_attr_destroy(&_attr);
}

uint32_t CThread::get_current_thread_id() throw ()
{
    return pthread_self();
}

void CThread::start(bool detach) throw (utils::CException, CSyscallException)
{
    int errcode = 0;

    // 回调
    before_start();

    // 设置线程栈大小
    if (_stack_size > 0)
    {
        errcode = pthread_attr_setstacksize(&_attr, _stack_size);
        if (errcode != 0)
            THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_attr_setstacksize");
    }

    errcode = pthread_attr_setdetachstate(&_attr, detach?PTHREAD_CREATE_DETACHED:PTHREAD_CREATE_JOINABLE);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_attr_setdetachstate");

    // 如果本过程成功，则线程体run结束后再减引用计数，
    // 否则在失败的分支减引用计数
    this->inc_refcount();

    errcode = pthread_create(&_thread , &_attr, thread_proc, this);
    if (errcode != 0)
    {
        this->dec_refcount();
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_create");
    }
}

size_t CThread::get_stack_size() const throw (CSyscallException)
{
    size_t stack_size = 0;
    int errcode = pthread_attr_getstacksize(&_attr, &stack_size);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_attr_getstacksize");

    return stack_size;
}

void CThread::join() throw (CSyscallException)
{
    // 线程自己不能调用join
    if (CThread::get_current_thread_id() != this->get_thread_id())
    {    
        if (_thread > 0)
        {
            int errcode = pthread_join(_thread, NULL);
            if (errcode != 0)
                THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_join");
        }
    }
}

void CThread::detach() throw (CSyscallException)
{
    int errcode = pthread_detach(_thread);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_detach");
}

bool CThread::can_join() const throw (CSyscallException)
{
    int detachstate;
    int errcode = pthread_attr_getdetachstate(&_attr, &detachstate);
    if (errcode != 0)
        THROW_SYSCALL_EXCEPTION(NULL, errcode, "pthread_attr_getdetachstate");

    return (PTHREAD_CREATE_JOINABLE == detachstate);
}

bool CThread::is_stop() const throw ()
{
    return _stop;
}

void CThread::do_wakeup(bool stop) throw (CSyscallException)
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

void CThread::stop(bool wait_stop) throw (utils::CException, CSyscallException)
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

void CThread::do_millisleep(int milliseconds) throw (CSyscallException)
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
