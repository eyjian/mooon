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
#ifndef MOOON_SYS_THREAD_POOL_H
#define MOOON_SYS_THREAD_POOL_H
#include "mooon/sys/utils.h"
SYS_NAMESPACE_BEGIN

/***
  * 线程池模板类，模板参数为线程类
  */
template <class ThreadClass>
class CThreadPool
{
public:
    /** 构造一个线程池 */
    CThreadPool() throw ()
        :_next_thread(0)
        ,_thread_count(0)
        ,_thread_array(NULL)
    {
    }

    /** 创建线程池，并启动线程池中的所有线程，
      * 池线程创建成功后，并不会立即进行运行状态，而是处于等待状态，
      * 所以需要唤醒它们，用法请参见后面的示例
      * @thread_count: 线程池中的线程个数
      * @parameter: 传递给池线程的参数
      * @exception: 可抛出CSyscallException异常，
      *             如果是因为CPoolThread::before_start返回false，则出错码为0
      */
    void create(uint16_t thread_count, void* parameter=NULL) throw (CSyscallException)
    {
        _thread_array = new ThreadClass*[thread_count];
        for (uint16_t i=0; i<thread_count; ++i)
        {
            _thread_array[i] = new ThreadClass;            
            _thread_array[i]->inc_refcount();
            _thread_array[i]->set_index(i);
            _thread_array[i]->set_parameter(parameter);
        }
        for (uint16_t i=0; i<thread_count; ++i)
        {
            try
            {                
                _thread_array[i]->start();				
                ++_thread_count;
            }
            catch (...)
            {
                destroy();
                throw;
            }
        }
    }

    /** 销毁线程池，这里会等待所有线程退出，然后删除线程 */
    void destroy() throw (CSyscallException)
    {
        if (_thread_array != NULL)
        {
            uint16_t thread_count = _thread_count;
            for (uint16_t i=thread_count; i>0; --i)
            {
                _thread_array[i-1]->stop();
                _thread_array[i-1]->dec_refcount();
                --_thread_count;
            }

            delete []_thread_array;
            _thread_array = NULL;
        }
    }

    /***
      * 激活线程池，将池中的所有线程唤醒，
      * 也可以单独调用各池线程的wakeup将它们唤醒
      */
    void activate() throw (CSyscallException)
    {
        for (uint16_t i=0; i<_thread_count; ++i)
            _thread_array[i]->wakeup();
    }

    /** 得到线程池中的线程个数 */
    uint16_t get_thread_count() const throw () { return _thread_count; }

    /** 得到线程池中的线程数组 */
    ThreadClass** get_thread_array() throw () { return _thread_array; }
    ThreadClass** get_thread_array() const throw () { return _thread_array; }

    /** 根据线程编号，得到对应的线程 */
    ThreadClass* get_thread(uint16_t thread_index) throw ()
    {
        if (0 == _thread_count) return NULL;
        if (thread_index > _thread_count) return NULL;
        return _thread_array[thread_index];
    }
    
    /** 根据线程编号，得到对应的线程 */
    ThreadClass* get_thread(uint16_t thread_index) const throw ()
    {
        if (0 == _thread_count) return NULL;
        if (thread_index > _thread_count) return NULL;
        return _thread_array[thread_index];
    }
    
    /***
      * 得到指向下个线程的指针，从第一个开始循环遍历，无终结点，即达到最后一个时，又指向第一个，
      * 主要应用于采用轮询方式将一堆任务分配均衡分配给池线程。
      */
    ThreadClass* get_next_thread() throw ()
    {
        if (0 == _thread_count) return NULL;
        if (_next_thread >= _thread_count)
            _next_thread = 0;

        return _thread_array[_next_thread++];
    }

private:    
    uint16_t _next_thread;
    uint16_t _thread_count;    
    ThreadClass** _thread_array;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_THREAD_POOL_H
