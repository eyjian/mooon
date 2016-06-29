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
 * 20110326: 通过增加一个空闲的头结点，简化CListQueue实现
 */
#ifndef MOOON_UTILS_LIST_QUEUE_H
#define MOOON_UTILS_LIST_QUEUE_H
#include "mooon/utils/listable.h"
UTILS_NAMESPACE_BEGIN

/***
  * 可链表对象模板队列，非线程安全类
  */
template <class ListableClass>
class CListQueue
{
public:
    /** 构造一个可链表对象模板队列 */
    CListQueue()
        :_number(0)
        ,_head(NULL)
        ,_tail(NULL)
    {
        _head = new ListableClass;
        _tail = _head;
    }

    ~CListQueue()
    {
        delete _head;
        _tail = NULL;
    }

    /** 得到队列中元素个数 */
    int get_number() const
    {
        return _number;
    }

    /** 判断队列是否为空 */
    bool is_empty() const
    {
        return 0 == _number;
    }

    /** 得到指向队首对象的指针 */
    ListableClass* front() const
    { 
        return (ListableClass*)_head->get_next();
    }

    /** 在队尾添加一个可链表对象 */
    void push(ListableClass* listable)
    {
        MOOON_ASSERT(listable != NULL);
        if (NULL == listable) return;
                
        ListableClass* next = listable->get_next();
        ListableClass* prev = listable->get_prev();
        if (prev != NULL) return; // 已经在队列中
        MOOON_ASSERT(NULL == next);
        
        listable->set_next(NULL);
        listable->set_prev(_tail);
        _tail->set_next(listable);
        _tail = listable;
        ++_number;
    }

    /** 
      * 将一个可链表对象从队列中删除
      * 删除操作是高效的，因为0查找，只需要解除链接关系即可
      */
    void remove(ListableClass* listable)
    {
        MOOON_ASSERT(listable != NULL);
        if (NULL == listable) return;
        
        ListableClass* next = listable->get_next();
        ListableClass* prev = listable->get_prev();
        if (NULL == prev) 
        {
            MOOON_ASSERT(NULL == next);
            return; // 已经不在队列中
        }
         
        prev->set_next(next);
        if (NULL == next)
        {
            // 尾结点的next才会为NULL
            _tail = prev;
        }
        else
        {
            next->set_prev(prev);
        }

        MOOON_ASSERT(_number > 0);
        listable->set_prev(NULL);
        listable->set_next(NULL);
        --_number;
    }

private:
    int _number;
    ListableClass* _head;
    ListableClass* _tail;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_LIST_QUEUE_H
