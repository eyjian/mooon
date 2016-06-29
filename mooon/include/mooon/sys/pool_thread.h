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
#ifndef MOOON_SYS_POOL_THREAD_H
#define MOOON_SYS_POOL_THREAD_H
#include "mooon/sys/event.h"
#include "mooon/sys/syscall_exception.h"
#include "mooon/sys/thread.h"
SYS_NAMESPACE_BEGIN

// 用于线程池的线程抽象基类
class CPoolThread: public CRefCountable
{
private:
    friend class CPoolThreadHelper;
    template <class ThreadClass> friend class CThreadPool;
    class CPoolThreadHelper: public CThread
    {
    public:
        CPoolThreadHelper(CPoolThread* pool_thread);
        void millisleep(int milliseconds) throw (CSyscallException);

    private:
        virtual void run();
        virtual void before_start() throw (utils::CException, CSyscallException);
        virtual void before_stop() throw (utils::CException, CSyscallException);

    private:		
        CPoolThread* _pool_thread;
    };

protected: // 禁止直接创建CPoolThread的实例
    CPoolThread() throw (utils::CException, CSyscallException);
    virtual ~CPoolThread();
    /***
      * 毫秒级sleep，线程可以调用它进入睡眠状态，并且可以通过调用wakeup唤醒，
      * 请注意只本线程可以调用此函数，其它线程调用无效
      */
    void do_millisleep(int milliseconds) throw (CSyscallException);

private:    
    /***
      * 线程执行体
      */
    virtual void run() = 0;

    /***
      * 在run之前被调用
      * @return 如果返回true，则会进入run，否则线程直接退出
      */
    virtual bool before_run() throw () { return true; }

    /***
      * 在run之后被调用
      */
    virtual void after_run() throw () { }

    /***
      * start执行前被调用
      */
    virtual void before_start() throw (utils::CException, CSyscallException) {}

    /***
      * stop执行前可安插的动作
      */
    virtual void before_stop() throw (utils::CException, CSyscallException) {}

    /** 设置线程在池中的顺序号 */
    void set_index(uint16_t index) { _index = index; }    
    
public:
    /** 设置参数，在before_start之前被回调 */
    virtual void set_parameter(void* parameter) { }
    
public:    
    /***
      * 唤醒池线程，池线程启动后，都会进入睡眠状态，
      * 直接调用wakeup将它唤醒
      */
    void wakeup() throw (CSyscallException);

    /***
      * 得到池线程在线程池中的序号，序号从0开始，
      * 且连续，但总是小于线程个数值。
      */
    uint16_t get_index() const throw () { return _index; }

    /** 设置线程栈大小，应当在before_start中设置。
      * @stack_size: 栈大小字节数
      * @exception: 不抛出异常
      */
    void set_stack_size(size_t stack_size) throw ();
    
    /** 得到线程栈大小字节数
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    size_t get_stack_size() const throw (CSyscallException);

    /** 得到本线程号 */
    uint32_t get_thread_id() const throw ();

private:
    void start() throw (CSyscallException);  /** 仅供CThreadPool调用 */
    void stop() throw (CSyscallException);   /** 仅供CThreadPool调用 */
	
private:	
    uint16_t _index;  /** 池线程在池中的位置 */
    CPoolThreadHelper* _pool_thread_helper;	
};


SYS_NAMESPACE_END
#endif // MOOON_SYS_POOL_THREAD_H
