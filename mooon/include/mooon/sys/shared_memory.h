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
#ifndef MOOON_SYS_SHARED_MEMORY_H
#define MOOON_SYS_SHARED_MEMORY_H
#include "mooon/sys/syscall_exception.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
SYS_NAMESPACE_BEGIN

/** System V共享内存C++包装类 */
class CSysVSharedMemory
{
public:
    CSysVSharedMemory() throw ();
    /***
      * 打开一个已经存在的共享内存
      * @path: 用来创建共享内存的路径(包含文件名)，不能为NULL
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void open(const char* path) throw (CSyscallException);

    /***
      * 创建一个共享内存
      * @path: 用来创建共享内存的路径(包含文件名)，如果为NULL则由系统选择独一无二的
      * @mode: 权限模式，其值为S_IRWXU和S_IXUSR等
      * @return: 如果已经存在则返回false，否则返回true
      * @exception: 如果出错则抛出CSyscallException异常
      */
    bool create(const char* path, mode_t mode=IPC_DEFAULT_PERM) throw (CSyscallException);
    
    /***
      * 关闭已经创建或打开的共享内存，
      * 如果已经没有进程关联到此共享内存，则删除它
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void close() throw (CSyscallException);

    /***
      * 解除和共享内存的关联，将共享内存从进程空间中移除
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void detach() throw (CSyscallException);

    /***
      * 关联共享内存，将共享内存映射到进程空间
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void* attach(int flag) throw (CSyscallException);

    /***
      * 得到共享内存的地址
      * @return: 如果已经关联则返回指向共享内存的地址，否则返回NULL
      */
    void* get_shared_memoty_address();
    void* get_shared_memoty_address() const;

private:
    int _shmid;
    void* _shmaddr; /** attach的共享内存地址 */
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SHARED_MEMORY_H
