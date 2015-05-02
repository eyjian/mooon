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
#include "sys/shared_memory.h"
SYS_NAMESPACE_BEGIN

CSysVSharedMemory::CSysVSharedMemory() throw ()
    :_shmid(-1)
    ,_shmaddr(NULL)
{
}

void CSysVSharedMemory::open(const char* path) throw (CSyscallException)
{
    if (NULL == path)
        THROW_SYSCALL_EXCEPTION(NULL, EINVAL, NULL);

    key_t key = ftok(path, getpid());
    if (-1 == key)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "ftok");

    _shmid = shmget(key, 1, 0);
    if (-1 == _shmid)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "shmget");
}

bool CSysVSharedMemory::create(const char* path, mode_t mode) throw (CSyscallException)
{
    key_t key = IPC_PRIVATE;
    
    // 得到IPC键
    if (path != NULL)
    {    
        key_t key = ftok(path, getpid());
        if (-1 == key)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "ftok");
    }

    // 创建共享内存
    _shmid = shmget(key, 1, IPC_CREAT | IPC_EXCL | mode);
    if (-1 == _shmid)
    {
        if (EEXIST == errno) return false;
        THROW_SYSCALL_EXCEPTION(NULL, errno, "shmget");
    }
        
    return true;
}

void CSysVSharedMemory::close() throw (CSyscallException)
{
    if (_shmid != -1)
    {    
        //struct shmid_ds buf;
        if (-1 == shmctl(_shmid, IPC_RMID, NULL))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "shmctl");

        _shmid = -1;
    }
}

void CSysVSharedMemory::detach() throw (CSyscallException)
{
    if (_shmaddr != NULL)
    {
        shmdt(_shmaddr);
        _shmaddr = NULL;
    }
}

void* CSysVSharedMemory::attach(int flag) throw (CSyscallException)
{
    if (_shmaddr != NULL)
    {    
        _shmaddr = shmat(_shmid, NULL, flag);
        if ((void *)-1 == _shmaddr)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "shmat");
    }

    return _shmaddr;
}

void* CSysVSharedMemory::get_shared_memoty_address()
{
    return _shmaddr;
}

void* CSysVSharedMemory::get_shared_memoty_address() const
{
    return _shmaddr;
}

SYS_NAMESPACE_END
