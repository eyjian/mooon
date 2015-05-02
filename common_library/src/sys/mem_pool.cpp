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
#include <utils/bit_utils.h>
#include "sys/mem_pool.h"
SYS_NAMESPACE_BEGIN

CRawMemPool::CRawMemPool() throw ()
    :_use_heap(false)
    ,_guard_size(0)
    ,_bucket_size(0)
    ,_bucket_number(0)   
    ,_stack_top_index(0)
    ,_available_number(0)
    ,_stack_top(NULL)
    ,_stack_bottom(NULL)
    ,_bucket_stack(NULL)
    ,_bucket_bitmap(NULL)
{
}

CRawMemPool::~CRawMemPool() throw ()
{
    destroy();
}

void CRawMemPool::destroy() throw ()
{
    _use_heap = false;
    _guard_size = 0;
    _bucket_size = 0;
    _bucket_number = 0;
    _stack_top_index = 0;
    _available_number = 0;

    if (_stack_bottom != NULL)
    {
        delete []_stack_bottom;
        _stack_bottom = NULL;
    }
    if (_bucket_stack != NULL)
    {
        delete []_bucket_stack;
        _bucket_stack = NULL;
    }
    if (_bucket_bitmap != NULL)
    {
        delete []_bucket_bitmap;
        _bucket_bitmap = NULL;
    }
}

void CRawMemPool::create(uint16_t bucket_size, uint32_t bucket_number, bool use_heap, uint8_t guard_size, char guard_flag) throw ()
{
    // 释放之前已经创建的
    destroy();

    // 保存对象大小和个数值
    _use_heap = use_heap;
    _guard_size = guard_size;
    _bucket_size = (bucket_size > 0)? bucket_size: 1;
    _bucket_number = (bucket_number > 0)? bucket_number: 1;

    // 有了guard_size更容易分析出是否有内存越界之类的行为
    _bucket_size += _guard_size;
    
    _bucket_stack = new char*[bucket_number];
    _stack_bottom = new char[_bucket_size * bucket_number];
    _stack_top = _stack_bottom + _bucket_size * (bucket_number - 1);
    _stack_top_index = bucket_number;
    _available_number = bucket_number;

    // 设置警戒标识
    memset(_stack_bottom, guard_flag, bucket_number * _bucket_size);
    
    for (uint32_t i=0; i<_bucket_number; ++i)    
        _bucket_stack[i] = _stack_bottom + _bucket_size * i; 
        
    // 初始化为1，加8是为了不四舍五入
    _bucket_bitmap = new char[(bucket_number+8) / 8];
    memset(_bucket_bitmap, 1, (bucket_number+8) / 8);
}

void* CRawMemPool::allocate() throw ()
{
    if (0 == _stack_top_index)
    {
        return _use_heap? new char[_bucket_size]: NULL;
    }
    else
    {        
        --_available_number;
        char*  ptr = _bucket_stack[--_stack_top_index];
        uint32_t bitmap_index = (ptr - _stack_bottom) / _bucket_size;

        utils::CBitUtils::set_bit(_bucket_bitmap, bitmap_index, false);  
        return ptr;
    }
}

bool CRawMemPool::reclaim(void* bucket) throw ()
{
    char* ptr = (char*)bucket;

    if ((ptr < _stack_bottom) || (ptr > _stack_top))
    {
        if (_use_heap)
        {
            delete []ptr;
            return true;
        }
        
        return false;
    }
    if ((ptr - _stack_bottom) % _bucket_size != 0)
    {
        // 边界不对
        return false;
    }

    uint32_t bitmap_index = (ptr - _stack_bottom) / _bucket_size;
    if (utils::CBitUtils::test(_bucket_bitmap, bitmap_index))
    {
        ++_available_number;
        _bucket_stack[_stack_top_index++] = ptr;
        utils::CBitUtils::set_bit(_bucket_bitmap, bitmap_index, true); 
    }

    return true;
}

bool CRawMemPool::use_heap() const throw ()
{
    return _use_heap;
}

uint8_t CRawMemPool::get_guard_size() const throw ()
{
    return _guard_size;
}

uint32_t CRawMemPool::get_pool_size() const throw ()
{
    return _bucket_number;
}

uint16_t CRawMemPool::get_bucket_size() const throw ()
{
    return _bucket_size;
}

uint32_t CRawMemPool::get_available_number() const throw ()
{
    return _available_number;
}

//////////////////////////////////////////////////////////////////////////
// CThreadMemPool

void CThreadMemPool::destroy() throw (CSyscallException)
{
    LockHelper<CLock> lock_helper(_lock);
    _raw_mem_pool.destroy();
}

void CThreadMemPool::create(uint16_t bucket_size, uint32_t bucket_number, bool use_heap, uint8_t guard_size, char guard_flag) throw (CSyscallException)
{
    LockHelper<CLock> lock_helper(_lock);
    _raw_mem_pool.create(bucket_size, bucket_number, use_heap, guard_size, guard_flag);
}

void* CThreadMemPool::allocate() throw (CSyscallException)
{
    LockHelper<CLock> lock_helper(_lock);
    return _raw_mem_pool.allocate();
}

bool CThreadMemPool::reclaim(void* bucket) throw (CSyscallException)
{
    LockHelper<CLock> lock_helper(_lock);
    return _raw_mem_pool.reclaim(bucket);
}

bool CThreadMemPool::use_heap() const throw ()
{
    return _raw_mem_pool.use_heap();
}

uint8_t CThreadMemPool::get_guard_size() const throw ()
{
    return _raw_mem_pool.get_guard_size();
}

uint32_t CThreadMemPool::get_pool_size() const throw ()
{
    return _raw_mem_pool.get_pool_size();
}

uint16_t CThreadMemPool::get_bucket_size() const throw ()
{
    return _raw_mem_pool.get_bucket_size();
}

uint32_t CThreadMemPool::get_available_number() const throw ()
{
    return _raw_mem_pool.get_available_number();
}

SYS_NAMESPACE_END
