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
#ifndef MOOON_SYS_SEMAPHORE_H
#define MOOON_SYS_SEMAPHORE_H
#include "mooon/sys/syscall_exception.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
SYS_NAMESPACE_BEGIN

/** System V信号量C++包装类 */
class CSysVSemaphore
{
public:
    CSysVSemaphore() throw ();
    
    /***
      * 打开一个已经存在的信号量
      * @path: 用来创建信号量的路径(包含文件名)，不能为NULL
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void open(const char* path) throw (CSyscallException);

    /***
      * 创建一个信号量
      * @path: 用来创建信号量的路径(包含文件名)，如果为NULL则由系统选择独一无二的
      * @init_value: 初始值
      * @mode: 权限模式，其值为S_IRWXU和S_IXUSR等
      * @return: 如果已经存在则返回false，否则返回true
      * @exception: 如果出错则抛出CSyscallException异常
      */
    bool create(const char* path, int16_t init_value=0, mode_t mode=IPC_DEFAULT_PERM) throw (CSyscallException);
    
    /***
      * 删除信号量
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void remove() throw (CSyscallException);
    
    /*** 
      * 信号量V操作，对信号量进行增操作，如果相加结果信号量值不小于0，则不阻塞
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void verhoog(uint16_t value) throw (CSyscallException);

    /***
      * 信号量P操作，对信号量减操作，如果减后仍大于或等于0，则不阻塞，否则阻塞
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void passeren(uint16_t value) throw (CSyscallException);

    /***
      * 尝试进行信号量P操作
      * @return: 如果P操作成功返回true，否则返回false
      * @exception: 如果出错则抛出CSyscallException异常
      */
    bool try_passeren(uint16_t value) throw (CSyscallException);

    /***
      * 超时方式进行信号量P操作
      * @milliseconds: 等待超时的毫秒数
      * @return: 如果在指定的时间内成功，则返回true，否则返回false
      * @exception: 如果出错则抛出CSyscallException异常
      */
    bool timed_passeren(uint16_t value, int milliseconds) throw (CSyscallException);
    
private:
    bool semaphore_operation(int value, int flag, int milliseconds) throw (CSyscallException);
    
private:
    int _semid; /** 信号量句柄 */
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SEMAPHORE_H
