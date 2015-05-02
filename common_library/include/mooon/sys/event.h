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
#ifndef MOOON_SYS_EVENT_H
#define MOOON_SYS_EVENT_H
#include "mooon/sys/lock.h"
SYS_NAMESPACE_BEGIN

/***
  * 通知事件类
  * 常用于队列的实现，队列为空时则等待队列有数据，如果往队列push了数据
  * ，则唤醒等待线程去队列取数据
  */
class CEvent
{
public:
    /***
      * 构造通知事件实例
      * @exception: 出错抛出CSyscallException异常，通常可不捕获此异常
      */
    CEvent() throw (CSyscallException);
    ~CEvent() throw ();

    /***
      * 让调用者进入等待状态，直接到被唤醒
      * @exception: 出错抛出CSyscallException异常，通常可不捕获此异常
      */
    void wait(CLock& lock) throw (CSyscallException);

    /***
      * 让调用者进入等待状态，直接到被唤醒，或等待的时长超过指定的毫秒数
      * @return: 如果在指定的毫秒数之前被唤醒，则返回true，否则返回false
      * @exception: 出错抛出CSyscallException异常，通常可不捕获此异常
      */
    bool timed_wait(CLock& mutex, uint32_t millisecond) throw (CSyscallException);

    /***
      * 唤醒一个进入等待状态的线程，如果没有线程正处于等待状态，则唤醒动作忽略
      * 只有当signal调用发生在wait调用之后才有效
      * @exception: 出错抛出CSyscallException异常，通常可不捕获此异常
      */
    void signal() throw (CSyscallException);

    /***
      * 广播唤醒信号，将所有进入等待状态的线程全部唤醒，
      * 如果没有线程正处于等待状态，则唤醒动作忽略
      * 只有当broadcast调用发生在wait调用之后才有效
      * @exception: 出错抛出CSyscallException异常，通常可不捕获此异常
      */
    void broadcast() throw (CSyscallException);
    
private:
    pthread_cond_t _cond;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_EVENT_H
