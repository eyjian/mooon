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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_NET_EPOLLABLE_QUEUE_H
#define MOOON_NET_EPOLLABLE_QUEUE_H
#include "mooon/net/epollable.h"
#include "mooon/sys/event.h"
NET_NAMESPACE_BEGIN

/** 可以放入Epoll监控的队列
  * RawQueueClass为原始队列类名，如utils::CArrayQueue
  * 为线程安全类
  */
template <class RawQueueClass>
class CEpollableQueue: public CEpollable
{
    typedef typename RawQueueClass::_DataType DataType;
    
public:
    /** 构造一个可Epoll的队列，注意只可监控读事件，也就是队列中是否有数据
      * @queue_max: 队列最大可容纳的元素个数
      * @exception: 如果出错，则抛出CSyscallException异常
      */
    CEpollableQueue(uint32_t queue_max) throw (sys::CSyscallException)
        :_raw_queue(queue_max)
        ,_push_waiter_number(0)
    {
        if (-1 == pipe(_pipefd))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "pipe");

        set_fd(_pipefd[0]);
    }
    
    ~CEpollableQueue()
    {
        close();
    }

    /** 关闭队列 */
    virtual void close()
    {
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        if (_pipefd[0] != -1)
        {     
            // 让CEpollable来关闭_pipefd[0]，在CEpollable::close()中将会调用
            // before_close，以保持语义总是相同的
            CEpollable::close();
            //close_fd(_pipefd[0]);
            _pipefd[0] = -1;            
        }
        if (_pipefd[1] != -1)
        {        
            close_fd(_pipefd[1]);
            _pipefd[1] = -1;
        }
    }

    /** 判断队列是否已满 */
    bool is_full() const 
	{
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        return _raw_queue.is_full();
    }
    
    /** 判断队列是否为空 */
    bool is_empty() const 
	{
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        return _raw_queue.is_empty();
    }

    /***
      * 取队首元素
      * @elem: 存储取到的队首元素
      * @return: 如果队列为空，则返回false，否则返回true
      */
    bool front(DataType& elem) const 
	{
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        if (_raw_queue.is_empty()) return false;

        elem = _raw_queue.front();
        return true;
    }
    
	/***
      * 弹出队首元素
      * @elem: 存储弹出的队首元素
      * @return: 如果队列为空，则返回false，否则取到元素并返回true
      * @exception: 如果出错，则抛出CSyscallException异常
      */
    bool pop_front(DataType& elem) 
	{
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        return do_pop_front(elem);
    }

    void pop_front()
    {
        DataType elem;
        (void)pop_front(elem);
    }

    /***
      * 从队首依次弹出多个元素
      * @elem_array: 存储弹出的队首元素数组
      * @array_size: 输入和输出参数，存储实际弹出的元素个数
      * @exception: 如果出错，则抛出CSyscallException异常
      */
    void pop_front(DataType* elem_array, uint32_t& array_size)
    {
        uint32_t i = 0;
        sys::LockHelper<sys::CLock> lock_helper(_lock);

        for (;;)
        {            
            if (!do_pop_front(elem_array[i])) break;
            if (++i == array_size) break;
        }

        array_size = i;
    }
    
	/***
      * 向队尾插入一元素
      * @elem: 待插入到队尾的元素
      * @millisecond: 如果队列满，等待队列非满的毫秒数，如果为0则不等待，直接返回false
      * @return: 如果队列已经满，则返回false，否则插入成功并返回true
      * @exception: 如果出错，则抛出CSyscallException异常
      */
    bool push_back(DataType elem, uint32_t millisecond=0) throw (sys::CSyscallException)
	{
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        while (_raw_queue.is_full())
        {
            // 立即返回
            if (0 == millisecond) return false;

            // 超时等待
            utils::CountHelper<volatile int32_t> ch(_push_waiter_number);            
            if (!_event.timed_wait(_lock, millisecond)) 
            {
                return false;
            }
        }        

        char c = 'x';
        _raw_queue.push_back(elem);
        // write还有相当于signal的作用
        while (-1 == write(_pipefd[1], &c, sizeof(c)))
        {
            if (errno != EINTR)
                THROW_SYSCALL_EXCEPTION(NULL, errno, "write");
        }

        return true;
    }

    /** 得到队列中当前存储的元素个数 */
    uint32_t size() const 
	{ 
        sys::LockHelper<sys::CLock> lock_helper(_lock);
        return _raw_queue.size(); 
	}

private:
    bool do_pop_front(DataType& elem) throw (sys::CSyscallException)
    {            
        // 没有数据，也不阻塞，如果需要阻塞，应当使用事件队列CEventQueue
        if (_raw_queue.is_empty()) return false;

        char c;
        // read还有相当于CEvent::wait的作用
        while (-1 == read(_pipefd[0], &c, sizeof(c)))
        {
            if (errno != EINTR)
                THROW_SYSCALL_EXCEPTION(NULL, errno, "read");
        }

        elem = _raw_queue.pop_front();
        // 如果有等待着，则唤醒其中一个
        if (_push_waiter_number > 0) _event.signal();
        
        return true;
    }

private:
    int _pipefd[2]; /** 管道句柄 */
    sys::CEvent _event;
    mutable sys::CLock _lock;    
    RawQueueClass _raw_queue; /** 普通队列实例 */
    volatile int32_t _push_waiter_number; /** 等待队列非满的线程个数 */
};

NET_NAMESPACE_END
#endif // MOOON_NET_EPOLLABLE_QUEUE_H
