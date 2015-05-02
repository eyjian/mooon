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
#ifndef MOOON_SYS_OBJECT_POOL_H
#define MOOON_SYS_OBJECT_POOL_H
#include <mooon/utils/array_queue.h>
#include "mooon/sys/lock.h"
SYS_NAMESPACE_BEGIN

/***
  * 池对象基类
  * 所以需要对象池的类都应当从它继承而来
  */
class CPoolObject
{
public:
    /** 构造一个池对象 */
    CPoolObject() throw ()
        :_in_pool(false)
        ,_index(0)
    {
    }

    /** 设置池对象的在池对象数组中的序号，0表示不是池中的对象 */
    void set_index(uint32_t index) throw ()
    {
        _index = index;
    }

    /** 得到池对象的在池对象数组中的序号 */
    uint32_t get_index() const throw ()
    {
        return _index;
    }

    /** 设置对象是否在池中的状态 */
    void set_in_pool(bool in_pool) throw ()
    {
        _in_pool = in_pool;
    }

    /** 判断池对象是否在池中 */
    bool is_in_pool() const throw ()
    {
        return _in_pool;
    }

private:
    bool _in_pool;
    uint32_t _index;    
};

/***
  * 裸对象池实现，性能高但非线程安全
  * 要求ObjectClass类必须是CPoolObject的子类
  */
template <class ObjectClass>
class CRawObjectPool
{
public:
    /***
      * 构造一个非线程安全的裸对象池
      * @use_heap: 当对象池中无对象时，是否从堆中创建对象
      */
    CRawObjectPool(bool use_heap) throw ()
        :_use_heap(use_heap)
        ,_object_number(0)
        ,_avaliable_number(0)
        ,_object_array(NULL)
        ,_object_queue(NULL)
    {
    }

    /** 析构裸对象池 */
    ~CRawObjectPool() throw ()
    {     
        destroy();
    }

    /***
      * 创建对象池
      * @object_number: 需要创建的对象个数
      */
    void create(uint32_t object_number) throw ()
    {
        _object_number = object_number;
        _avaliable_number = _object_number;

        _object_array = new ObjectClass[_object_number];
        _object_queue = new utils::CArrayQueue<ObjectClass*>(_object_number);
        
        for (uint32_t i=0; i<_object_number; ++i)
        {
            ObjectClass* object = &_object_array[i];
            object->set_index(i+1); // Index总是大于0，0作为无效标识
            object->set_in_pool(true);

            _object_queue->push_back(object);
        }
    }

    /** 销毁对象池 */
    void destroy() throw ()
    {
        delete _object_queue;
        delete []_object_array;

        _object_queue = NULL;
        _object_array = NULL;
    }

    /***
      * 从对象池中借用一个对象，并将对象是否在池中的状态设置为false
      * @return: 如果对象池为空，但允许从堆中创建对象，则从堆上创建一个新对象，并返回它，
      *          如果对象池为空，且不允许从堆中创建对象，则返回NULL，
      *          如果对象池不为空，则从池中取一个对象，并返回指向这个对象的指针
      */
    ObjectClass* borrow() throw ()
    {
        ObjectClass* object = NULL;
        
        // 如果队列为空，则看是否从堆中创建新对象，如果不可以，则返回NULL
        if (_object_queue->is_empty()) 
        {
            if (_use_heap)
            {
                object = new ObjectClass;
                object->set_index(0); // index为0，表示不是对象池中的对象
            }
        }
        else
        {
            object = _object_queue->pop_front();
            object->set_in_pool(false);
            --_avaliable_number;
        }        

        return object;
    }

    /***
      * 将一个对象归还给对象池
      * @object: 指向待归还给对象池的对象指针，如果对象并不是对象池中的对象，则delete它，
      *          否则将它放回对象池，并将是否在对象池中的状态设置为true
      */
    void pay_back(ObjectClass* object) throw ()
    {
        // 如果是对象池中的对象
        if (0 == object->get_index())
        {       
            delete object;
        }
        else
        {
            // 如果不在队列中
            if (!object->is_in_pool())
            {
                object->reset();
                object->set_in_pool(true);

                _object_queue->push_back(object);
                ++_avaliable_number;
            }
        }
    }

    /** 得到总的对象个数，包括已经借出的和未借出的 */
    uint32_t get_pool_size() const throw ()
    {
        return _object_number;
    }
    
    /** 得到对象池中还未借出的对象个数 */
    volatile uint32_t get_avaliable_number() const throw ()
    {
        return _avaliable_number;
    }

private:
    bool _use_heap;
    uint32_t _object_number;
    volatile uint32_t _avaliable_number;
    ObjectClass* _object_array;
    utils::CArrayQueue<ObjectClass*>* _object_queue;
};

/***
  * 线程安全的对象池，性能较CRawObjectPool低
  * 要求ObjectClass类必须是CPoolObject的子类
  */
template <class ObjectClass>
class CThreadObjectPool
{
public: 
    /***
      * 构造一个线程安全的裸对象池
      * @use_heap: 当对象池中无对象时，是否从堆中创建对象
      */
    CThreadObjectPool(bool use_heap) throw (CSyscallException)
        :_raw_object_pool(use_heap)
    {        
    }   
    
    /***
      * 创建对象池
      * @object_number: 需要创建的对象个数
      */
    void create(uint32_t object_number) throw (CSyscallException)
    {
        LockHelper<CLock> lock_helper(_lock);
        _raw_object_pool.create(object_number);
    }

    /** 销毁对象池 */
    void destroy() throw (CSyscallException)
    {
        LockHelper<CLock> lock_helper(_lock);
        _raw_object_pool.destroy();
    }

    /** 向对象池借用一个对象 */
    ObjectClass* borrow() throw (CSyscallException)
    {
        LockHelper<CLock> lock_helper(_lock);
        return _raw_object_pool.borrow();
    }

    /** 将一个对象归还给对象池 */
    void pay_back(ObjectClass* object) throw (CSyscallException)
    {
        LockHelper<CLock> lock_helper(_lock);
        _raw_object_pool.pay_back(object);
    }

    /** 得到总的对象个数，包括已经借出的和未借出的 */
    uint32_t get_pool_size() const throw (CSyscallException)
    {
        LockHelper<CLock> lock_helper(_lock);
        return _raw_object_pool.get_pool_size();
    }
    
    /** 得到对象池中还未借出的对象个数 */
    volatile uint32_t get_avaliable_number() const throw (CSyscallException)
    {
        LockHelper<CLock> lock_helper(_lock);
        return _raw_object_pool.get_avaliable_number();
    }
    
private:
    CLock _lock;
    CRawObjectPool<ObjectClass> _raw_object_pool;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_OBJECT_POOL_H
