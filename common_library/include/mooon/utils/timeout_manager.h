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
#ifndef MOOON_UTILS_TIMEOUT_MANAGER_H
#define MOOON_UTILS_TIMEOUT_MANAGER_H
#include "mooon/utils/list_queue.h"
#include "mooon/utils/timeoutable.h"
UTILS_NAMESPACE_BEGIN

/***
  * 超时处理器抽象接口
  * 发生超时时，回调它的on_timeout_event方法
  */
template <class TimeoutableClass>
class CALLBACK_INTERFACE ITimeoutHandler
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~ITimeoutHandler() {}
    /** 超时时被回调 */
    virtual void on_timeout_event(TimeoutableClass* timeoutable) = 0;
};

/***
  * 超时管理模板类，维护一个超时队列，提供超时检测方法
  * TimeoutableClass要求为CTimeoutable的子类型
  * 队列中的所有对象总是按时间先后顺序进入的，因此只需要从队首开始检测哪些超时了
  * 非线程安全类，因此通常一个线程一个CTimeoutManager实例，而
  * TimeoutableClass类型的对象通常也不跨线程，
  * 这保证高效的前提，使得整个超时检测0查找
  */
template <class TimeoutableClass>
class CTimeoutManager
{
public: 
    /*** 无参数构造函数 */
    CTimeoutManager()
        :_timeout_seconds(0)
        ,_timeout_handler(NULL)
    {
    }
    
    /** 得到超时秒数 */
    uint32_t get_timeout_seconds() const
    {
        return _timeout_seconds;
    }

    /** 设置超时秒数，也就是在这个时长内不算超时 */
    void set_timeout_seconds(uint32_t timeout_seconds)
    {
        _timeout_seconds = timeout_seconds;
    }

    /** 设置超时处理器 */
    void set_timeout_handler(ITimeoutHandler<TimeoutableClass>* timeout_handler)
    {
        _timeout_handler = timeout_handler;
    }

    /***
      * 将超时对象插入到超时队列尾
      * @timeoutable: 指向可超时的对象指针
      * @current_time: 当前时间
      */
    void push(TimeoutableClass* timeoutable, time_t current_time)
    {
        timeoutable->set_timestamp(current_time);
        _list_queue.push(timeoutable);
    }

    /***
      * 将一个可超时对象从队列中删除
      * @timeoutable: 指向可超时的对象指针
      */
    void remove(TimeoutableClass* timeoutable)
    {
        timeoutable->set_timestamp(0);
        _list_queue.remove(timeoutable);
    }    

    TimeoutableClass* pop_front()
    {
        TimeoutableClass* timeoutable = NULL;

        if (!_list_queue.is_empty())
        {
            timeoutable = _list_queue.front();
            _list_queue.remove(timeoutable);
        }

        return timeoutable;
    }

    /***
      * 重新排队
      */
    void update(TimeoutableClass* timeoutable, time_t current_time)
    {
        remove(timeoutable);
        push(timeoutable, current_time);
    }

    /***
      * 检测队列中哪些对象发生了超时
      * @current_time: 当前时间
      * 说明: 从队首开始循环遍历哪些对象发生了超时，如果超时发生，则
      * 回调ITimeoutHandler的on_timeout_event方法，
      * 直接检测到某对象未超时，便利结束
      */
    void check_timeout(time_t current_time)
    {
        for (;;)
        {
            TimeoutableClass* timeoutable = _list_queue.front();
            if (NULL == timeoutable) break;

            time_t last_time = timeoutable->get_timestamp();
            if (!is_timeout(last_time, current_time)) break;
          
            _list_queue.remove(timeoutable);          
            _timeout_handler->on_timeout_event(timeoutable);            
        }
    }

private:
    inline bool is_timeout(time_t last_time, time_t current_time)
    {
        return last_time + _timeout_seconds < current_time;
    }
    
private:
    time_t _timeout_seconds;
    ITimeoutHandler<TimeoutableClass>* _timeout_handler;    
    typename utils::CListQueue<TimeoutableClass> _list_queue;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_TIMEOUT_MANAGER_H
