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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_UTILS_ARRAY_QUEUE_H
#define MOOON_UTILS_ARRAY_QUEUE_H
#include "mooon/utils/config.h"
UTILS_NAMESPACE_BEGIN

/** 用数组实现的队列, 非线程安全 */
template <typename DataType>
class CArrayQueue
{       
public:
    /** 队列中的元素数据类型 */
    typedef DataType _DataType;

    /***
      * 构造一个数组队列
      * @queue_max: 需要构造的队列大小
      */
    CArrayQueue(uint32_t queue_max)
        :_tail(0)
        ,_head(0)
        ,_queue_size(0)
    {
        _queue_max = queue_max;
        if (0 == _queue_max)
        {
            _elem_array = NULL;
        }
        else
        {
            _elem_array = new DataType[_queue_max];        
            memset(_elem_array, 0, _queue_max);
        }
    }

    ~CArrayQueue()
    {
        delete []_elem_array;
    }

    /** 判断队列是否已满 */
    bool is_full() const 
    {
        return (_queue_max == _queue_size); //((_tail+1) % _queue_max == _head);
    }
    
    /** 判断队列是否为空 */
    bool is_empty() const 
    {
        return (0 == _queue_size); //(_head == _tail);
    }

    /** 返回队首元素 */
    DataType front() const 
    {
        return _elem_array[_head];
    }
    
    /***
      * 弹出队首元素
      * 注意: 调用pop之前应当先使用is_empty判断一下
      * @return: 返回队首元素
      */
    DataType pop_front() 
    {
        DataType elem = _elem_array[_head];
        _head = (_head+1) % _queue_max;
        --_queue_size;
        return elem;
    }
    
    /***
      * 往队尾插入一个元素
      * 注意: 调用pop之前应当先使用is_full判断一下
      */
    void push_back(DataType elem) 
    {
        _elem_array[_tail] = elem;
        _tail = (_tail+1) % _queue_max; 
        ++_queue_size;
    }

    /** 得到队列中存储的元素个数 */
    uint32_t size() const 
    {
        return _queue_size;
    }
    
    /** 得到队列的容量 */
    uint32_t capacity() const
    {
        return _queue_max;
    }

private:        
    volatile uint32_t _tail;       /** 队首 */
    volatile uint32_t _head;       /** 队尾 */
    volatile uint32_t _queue_size; /** 队列当前容纳的元素个数 */
    uint32_t _queue_max;  /** 队列最多可容纳的元素个数 */
    DataType* _elem_array; /** 用来实现队列的数组 */
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_ARRAY_QUEUE_H
