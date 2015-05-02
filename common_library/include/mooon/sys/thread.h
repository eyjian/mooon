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
#ifndef MOOON_SYS_THREAD_H
#define MOOON_SYS_THREAD_H
#include "mooon/sys/event.h"
#include "mooon/sys/utils.h"
#include "mooon/sys/ref_countable.h"
#include <pthread.h>
SYS_NAMESPACE_BEGIN

/**
 * 线程抽象基类
 * 注意事项: 不能定义CThread的栈对象，只能为数堆对象，
 * 即不能CThread thread，而应当为CThread* thread = new CThread
 */
class CThread: public CRefCountable
{
public:
    /** 线程执行体函数，子类必须实现该函数，此函数内的代码半在一个独立的线程中执行 */
	virtual void run() = 0;
    
private:
    /** 在启动线程之前被调用的回调函数，如果返回false，则会导致start调用也返回false。
      * 可以重写该函数，将线程启动之前的逻辑放在这里。
      */
    virtual void before_start() throw (utils::CException, CSyscallException) {}

    /***
      * stop执行前可安插的动作
      */
    virtual void before_stop() throw (utils::CException, CSyscallException) {}

public:
    /** 得到当前线程号 */
    static uint32_t get_current_thread_id() throw ();

public:
	CThread() throw (utils::CException, CSyscallException);
	virtual ~CThread();

    /** 将_stop成员设置为true，线程可以根据_stop状态来决定是否退出线程
      * @wait_stop: 是否等待线程结束，只有当线程是可Join时才有效
      * @exception: 当wait_stop为true时，抛出和join相同的异常，否则不抛异常
      */
    virtual void stop(bool wait_stop=true) throw (utils::CException, CSyscallException);

    /** 启动线程
      * @detach: 是否以可分离模式启动线程
      * @exception: 如果失败，则抛出CSyscallException异常，
      *             如果是因为before_start返回false，则出错码为0
      */
	void start(bool detach=false) throw (utils::CException, CSyscallException);

    /** 设置线程栈大小。应当在start之前调用，否则设置无效，如放在before_start当中。
      * @stack_size: 栈大小字节数
      * @exception: 不抛出异常
      */
    void set_stack_size(size_t stack_size) throw () { _stack_size = stack_size; }
    
    /** 得到线程栈大小字节数
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    size_t get_stack_size() const throw (CSyscallException);

    /** 得到本线程号 */
    uint32_t get_thread_id() const { return _thread; }
    
    /** 等待线程返回
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    void join() throw (CSyscallException);

    /** 将线程设置为可分离的，
      * 请注意一旦将线程设置为可分离的，则不能再转换为可join。
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    void detach() throw (CSyscallException);

    /** 返回线程是否可join
      * @return: 如果线程可join则返回true，否则返回false
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    bool can_join() const throw (CSyscallException);

    /***
      * 如果线程正处于等待状态，则唤醒
      */
    void wakeup();

protected: // 仅供子类使用
    /***
      * 判断线程是否应当退出，默认返回_stop的值
      */
    virtual bool is_stop() const throw ();

    /***
      * 毫秒级sleep，线程可以调用它进入睡眠状态，并且可以通过调用do_wakeup唤醒，
      * 请注意只本线程可以调用此函数，其它线程调用无效
      */
    void do_millisleep(int milliseconds) throw (CSyscallException);

private:
    void do_wakeup(bool stop) throw (CSyscallException);

protected:
    CLock _lock; /** 同步锁，可供子类使用 */

private:        
    CEvent _event;
    volatile bool _stop; /** 是否停止线程标识 */
    /** 线程当前状态: 等待、唤醒和正在运行 */
    volatile enum { state_sleeping, state_wakeuped, state_running } _current_state;

private:
    pthread_t _thread;
    pthread_attr_t _attr;
    size_t _stack_size;    
};


SYS_NAMESPACE_END
#endif // MOOON_SYS_THREAD_H
