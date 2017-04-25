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
#ifndef MOOON_SYS_MMAP_H
#define MOOON_SYS_MMAP_H
#include "mooon/sys/utils.h"
SYS_NAMESPACE_BEGIN

typedef struct
{
    int fd;      /** 如果是通过fd映射的，则为文件句柄的负值，否则为mmap操作时打开的文件句柄 */
    void* addr;  /** 文件映射到内存的地址，如果文件未被映射到内存则为NULL */
    size_t len;  /** 计划映射到内存的大小 */
}mmap_t;

/** 内存文件映射操作类，所以成员均为静态类成员 */
class CMMap
{
public:
    /** 以只读方式将文件映射到内存
      * @fd: 文件句柄，调用者需要负责关闭此句柄，mmap_t结构的fd成员为它的负值
      * @size: 需要映射到内存的大小，如果为0则映射整个文件
      * @offset: 映射的偏移位置
      * @size_max: 最大可映射字节数，超过此大小的文件将不会被映射到内存，mmap_t结构的addr成员将为NULL
      * @return: 返回指向mmap_t结构的指针，返回值总是不会为NULL
      * @exception: 出错抛出CSyscallException异常
      */
    static mmap_t* map_read(int fd, size_t size=0, size_t offset=0, size_t size_max=0) throw (CSyscallException);

    /** 以只读方式将文件映射到内存
      * @filename: 文件名，mmap_t结构的fd成员为打开此文件的句柄
      * @size_max: 最大可映射字节数，超过此大小的文件将不会被映射到内存，mmap_t结构的addr成员将为NULL
      * @return: 返回指向mmap_t结构的指针，返回值总是不会为NULL
      * @exception: 出错抛出CSyscallException异常
      */
    static mmap_t* map_read(const char* filename, size_t size_max=0) throw (CSyscallException);

    /** 以只写方式将文件映射到内存
      * @fd: 文件句柄，调用者需要负责关闭此句柄，mmap_t结构的fd成员为它的负值
      * @size: 需要映射到内存的大小，如果为0则映射整个文件
      * @offset: 映射的偏移位置
      * @size_max: 最大可映射字节数，超过此大小的文件将不会被映射到内存，mmap_t结构的addr成员将为NULL
      * @return: 返回指向mmap_t结构的指针，返回值总是不会为NULL
      * @exception: 出错抛出CSyscallException异常
      */
    static mmap_t* map_write(int fd, size_t size=0, size_t offset=0, size_t size_max=0) throw (CSyscallException);

    /** 以只写方式将文件映射到内存
      * @filename: 文件名，mmap_t结构的fd成员为打开此文件的句柄
      * @size_max: 最大可映射字节数，超过此大小的文件将不会被映射到内存，mmap_t结构的addr成员将为NULL
      * @return: 返回指向mmap_t结构的指针，返回值总是不会为NULL
      * @exception: 出错抛出CSyscallException异常
      */
    static mmap_t* map_write(const char* filename, size_t size_max=0) throw (CSyscallException);

    /** 以读和写方式将文件映射到内存
      * @fd: 文件句柄，调用者需要负责关闭此句柄，mmap_t结构的fd成员为它的负值
      * @size: 需要映射到内存的大小，如果为0则映射整个文件
      * @offset: 映射的偏移位置
      * @size_max: 最大可映射字节数，超过此大小的文件将不会被映射到内存，mmap_t结构的addr成员将为NULL
      * @return: 返回指向mmap_t结构的指针，返回值总是不会为NULL
      * @exception: 出错抛出CSyscallException异常
      */
    static mmap_t* map_both(int fd, size_t size=0, size_t offset=0, size_t size_max=0) throw (CSyscallException);

    /** 以读和写方式将文件映射到内存
      * @filename: 文件名，mmap_t结构的fd成员为打开此文件的句柄
      * @size_max: 最大可映射字节数，超过此大小的文件将不会被映射到内存，mmap_t结构的addr成员将为NULL
      * @return: 返回指向mmap_t结构的指针，返回值总是不会为NULL
      * @exception: 出错抛出CSyscallException异常
      */
    static mmap_t* map_both(const char* filename, size_t size_max=0) throw (CSyscallException);

    /***
      * 释放已创建的内存映射，如果是通过指定文件名映射的，则关闭在mmap中打开的句柄
      * @ptr: 已创建的内存映射
      * @exception: 出错抛出CSyscallException异常
      */
    static void unmap(mmap_t* ptr) throw (CSyscallException);

    /***
      * 同步地将内存刷新到磁盘
      * @ptr: 指向map_read、map_both或map_write得到的mmap_t指针
      * @offset: 需要刷新的偏移位置
      * @length: 需要刷新的大小，如果为0，则表示从偏移处到最尾，
      *          如果超出边界，则只刷新到边界
      * @invalid: 是否标识内存为无效
      * @exception: 出错抛出CSyscallException异常
      */
    static void sync_flush(mmap_t* ptr, size_t offset=0, size_t length=0, bool invalid=false) throw (CSyscallException);

    /***
      * 异步地将内存刷新到磁盘
      * @ptr: 指向map_read、map_both或map_write得到的mmap_t指针
      * @offset: 需要刷新的偏移位置
      * @length: 需要刷新的大小，如果为0，则表示从偏移处到最尾，
      *          如果超出边界，则只刷新到边界
      * @invalid: 是否标识内存为无效
      * @exception: 出错抛出CSyscallException异常
      */
    static void async_flush(mmap_t* ptr, size_t offset=0, size_t length=0, bool invalid=false) throw (CSyscallException);

private:
    static mmap_t* do_map(int prot, int fd, size_t size, size_t offset, size_t size_max, bool byfd) throw (CSyscallException);
};

/***
  * CMMap帮助类，用于自动卸载已经映射的文件
  */
class MMapHelper
{
public:
    /** 构造一个CMmap帮助类 */
    MMapHelper(mmap_t* ptr)
        :_ptr(ptr)
    {        
    }

    /** 析构函数，用来自动卸载已经映射的文件 */
    ~MMapHelper()
    {
        if (_ptr != NULL)
            CMMap::unmap(_ptr);
    }

    mmap_t* operator ->()
    {
        return _ptr;
    }

private:
    mmap_t* _ptr;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_MMAP_H
