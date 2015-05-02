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
#include "sys/pool_thread.h"
SYS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// CPoolThread::CPoolThreadHelper

CPoolThread::CPoolThreadHelper::CPoolThreadHelper(CPoolThread* pool_thread)
    :_pool_thread(pool_thread)
{
}

void CPoolThread::CPoolThreadHelper::run()
{
	// wait用于和主线程同步
	do_millisleep(-1);

    if (!is_stop())
    {
        if (_pool_thread->before_run())
        {
            while (!is_stop())
            {
                _pool_thread->run();
            }

            _pool_thread->after_run();
        }
    }
}

void CPoolThread::CPoolThreadHelper::before_start() throw (utils::CException, CSyscallException)
{
    _pool_thread->before_start();
}

void CPoolThread::CPoolThreadHelper::before_stop() throw (utils::CException, CSyscallException)
{
    _pool_thread->before_stop();
}

void CPoolThread::CPoolThreadHelper::millisleep(int milliseconds) throw (CSyscallException)
{
    do_millisleep(milliseconds);
}

//////////////////////////////////////////////////////////////////////////
// CPoolThread

CPoolThread::CPoolThread() throw (utils::CException, CSyscallException)
	:_index(std::numeric_limits<uint16_t>::max())
{
    _pool_thread_helper = new CPoolThreadHelper(this);
    _pool_thread_helper->inc_refcount(); // 保证生命周期内都是可以用的
}

CPoolThread::~CPoolThread()
{
    _pool_thread_helper->dec_refcount();
}

void CPoolThread::wakeup() throw (CSyscallException)
{
	_pool_thread_helper->wakeup();
}

void CPoolThread::start() throw (CSyscallException)
{
    _pool_thread_helper->start();
}

void CPoolThread::stop() throw (CSyscallException)
{
    _pool_thread_helper->stop();    
}

void CPoolThread::set_stack_size(size_t stack_size) throw ()
{
    _pool_thread_helper->set_stack_size(stack_size);
}

size_t CPoolThread::get_stack_size() const throw (CSyscallException)
{
    return _pool_thread_helper->get_stack_size();
}

uint32_t CPoolThread::get_thread_id() const throw ()
{
    return _pool_thread_helper->get_thread_id();
}

void CPoolThread::do_millisleep(int milliseconds) throw (CSyscallException)
{
    _pool_thread_helper->millisleep(milliseconds);
}

SYS_NAMESPACE_END
