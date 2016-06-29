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
#ifndef MOOON_SYS_MEM_POOL_H
#define MOOON_SYS_MEM_POOL_H
#include "mooon/sys/lock.h"
SYS_NAMESPACE_BEGIN

/***
  * 裸内存池实现，性能高但非线程安全
  */
class CRawMemPool
{
public:
    CRawMemPool() throw ();
    ~CRawMemPool() throw ();

    /** 销毁由create创建的内存池 */
    void destroy() throw ();

    /***
      * 创建内存池
      * @bucket_size: 内存大小
      * @bucket_number: 内存个数
      * @use_heap: 内存池不够时，是否从堆上分配
      * @guard_size: 警戒大小
      * @guard_flag: 警戒标识
      */
    void create(uint16_t bucket_size, uint32_t bucket_number, bool use_heap=true, uint8_t guard_size=1, char guard_flag='m') throw ();

    /***
      * 分配内存内存
      * @return: 如果内存池不够，且设置了从堆上分配内存，则返回从堆上分配的内存，
      *          否则如果内存池不够时返回NULL，否则返回从内存池中分配的内存
      */
    void* allocate() throw ();

    /***
      * 回收内存内存
      * @bucket: 需要被回收到内存池中的内存，如果不是内存池中的内存，
      *          但create时允许从堆分配，则直接释放该内存，否则如果在池内存范围内，
      *          则检查是否为正确的池内存，如果是则回收并返回true，其它情况返回false
      * @return: 如果被回收或删除返回true，否则返回false
      */
    bool reclaim(void* bucket) throw ();

    /** 返回当内存池不够用时，是否从堆上分配内存 */
    bool use_heap() const throw ();

    /** 得到警戒值大小 */
    uint8_t get_guard_size() const throw ();

    /** 得到池大小，也就是池中可分配的内存个数 */
    uint32_t get_pool_size() const throw ();

    /** 得到内存池可分配的内存大小 */
    uint16_t get_bucket_size() const throw ();

    /** 得到内存池中，当前还可以分配的内存个数 */
    uint32_t get_available_number() const throw ();

private:    
    bool _use_heap;             /** 内存池不够时，是否从堆上分配 */
    uint8_t _guard_size;        /** 警戒大小，实际需要的内存大小为: (_guard_size+_bucket_size)*_bucket_number */
    uint16_t _bucket_size;      /** 内存大小，包含_guard_size部分，所以实际内存大小应当再减去_guard_size */
    uint32_t _bucket_number;    /** 内存个数 */
    volatile uint32_t _stack_top_index;  /** 栈顶索引 */
    volatile uint32_t _available_number; /** 池中还可以分配的内存个数 */

private:
    char* _stack_top;    
    char* _stack_bottom;
    char** _bucket_stack;
    char* _bucket_bitmap; /** 桶状态，用来记录当前状态，以防止重复回收 */
};

/***
  * 线程安全的内存池，性能较CRawMemPool要低
  */
class CThreadMemPool
{
public:
    CThreadMemPool() throw (CSyscallException) {}

    /** 销毁由create创建的内存池 */
    void destroy() throw (CSyscallException);

    /***
      * 创建内存池
      * @bucket_size: 内存大小
      * @bucket_number: 内存个数
      * @use_heap: 内存池不够时，是否从堆上分配
      * @guard_size: 警戒大小
      * @guard_flag: 警戒标识
      */
    void create(uint16_t bucket_size, uint32_t bucket_number, bool use_heap=true, uint8_t guard_size=1, char guard_flag='m') throw (CSyscallException);

    /***
      * 分配内存内存
      * @return: 如果内存池不够，且设置了从堆上分配内存，则返回从堆上分配的内存，
      *          否则如果内存池不够时返回NULL，否则返回从内存池中分配的内存
      */
    void* allocate() throw (CSyscallException);

    /***
      * 回收内存内存
      * @bucket: 需要被回收到内存池中的内存，如果不是内存池中的内存，
      *          但create时允许从堆分配，则直接释放该内存，否则如果在池内存范围内，
      *          则检查是否为正确的池内存，如果是则回收并返回true，其它情况返回false
      * @return: 如果被回收或删除返回true，否则返回false
      */
    bool reclaim(void* bucket) throw (CSyscallException);

    /** 返回当内存池不够用时，是否从堆上分配内存 */
    bool use_heap() const throw ();

    /** 得到警戒值大小 */
    uint8_t get_guard_size() const throw ();

    /** 得到池大小，也就是池中可分配的内存个数 */
    uint32_t get_pool_size() const throw ();

    /** 得到内存池可分配的内存大小 */
    uint16_t get_bucket_size() const throw ();

    /** 得到内存池中，当前还可以分配的内存个数 */
    uint32_t get_available_number() const throw ();
    
private:
    CLock _lock;
    CRawMemPool _raw_mem_pool;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_MEM_POOL_H
