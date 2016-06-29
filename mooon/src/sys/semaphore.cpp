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
#include "sys/semaphore.h"
SYS_NAMESPACE_BEGIN

union semun
{
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO(Linux-specific) */
};

CSysVSemaphore::CSysVSemaphore() throw ()
    :_semid(-1)
{
}

void CSysVSemaphore::open(const char* path) throw (CSyscallException)
{
    if (NULL == path)
        THROW_SYSCALL_EXCEPTION(NULL, EINVAL, NULL);

    key_t key = ftok(path, getpid());
    if (-1 == key)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "ftok");

    _semid = semget(key, 1, 0);
    if (-1 == _semid)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "semget");
}

bool CSysVSemaphore::create(const char* path, int16_t init_value, mode_t mode) throw (CSyscallException)
{
    key_t key = IPC_PRIVATE;
    
    // 得到IPC键
    if (path != NULL)
    {    
        key_t key = ftok(path, getpid());
        if (-1 == key)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "ftok");
    }

    // 创建信号量
    _semid = semget(key, 1, IPC_CREAT | IPC_EXCL | mode);
    if (-1 == _semid)
    {
        if (EEXIST == errno) return false;
        THROW_SYSCALL_EXCEPTION(NULL, errno, "semget");
    }

    // 设置信号量初始值
    union semun semopts;
    semopts.val = init_value;
    if (-1 == semctl(_semid, 0, SETVAL, &semopts))
    {
        remove();
        THROW_SYSCALL_EXCEPTION(NULL, errno, "semctl");
    }
        
    return true;
}

void CSysVSemaphore::remove() throw (CSyscallException)
{
    if (_semid != -1)
    {    
        if (-1 == semctl(_semid, 0, IPC_RMID))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "semctl");

        _semid = -1;
    }
}

void CSysVSemaphore::verhoog(uint16_t value) throw (CSyscallException)
{
    int op_val = value;
    (void)semaphore_operation(op_val, SEM_UNDO, -1);
}

void CSysVSemaphore::passeren(uint16_t value) throw (CSyscallException)
{
    int op_val = value;
    (void)semaphore_operation(-op_val, SEM_UNDO, -1);
}

bool CSysVSemaphore::try_passeren(uint16_t value) throw (CSyscallException)
{
    int op_val = value;
    return semaphore_operation(op_val, SEM_UNDO|IPC_NOWAIT, -1);
}

bool CSysVSemaphore::timed_passeren(uint16_t value, int milliseconds) throw (CSyscallException)
{
    int op_val = value;
    return semaphore_operation(op_val, SEM_UNDO, milliseconds);
}

bool CSysVSemaphore::semaphore_operation(int value, int flag, int milliseconds) throw (CSyscallException)
{
    /* If an operation specifies SEM_UNDO, 
       it will be automatically undone when the process terminates.
    */

    struct sembuf sops[1];
    sops[0].sem_num = 0;               /* semaphore number */
    sops[0].sem_op  = (int16_t)value;  /* semaphore operation */    
    sops[0].sem_flg = flag;            /* operation flags, such as SEM_UNDO */
    
    if (milliseconds < 0)
    {    
        if (-1 == semop(_semid, sops, sizeof(sops)/sizeof(sops[0])))
        {
            if (EAGAIN == errno) return false;
            THROW_SYSCALL_EXCEPTION(NULL, errno, "semop");
        }
    }
    else
    {
        struct timespec timeout;
        timeout.tv_sec  =  milliseconds / 1000;
        timeout.tv_nsec = (milliseconds % 1000) * 1000000;
        if (-1 == semtimedop(_semid, sops, sizeof(sops)/sizeof(sops[0]), &timeout))
        {
            if (errno == EAGAIN) return false;
            THROW_SYSCALL_EXCEPTION(NULL, errno, "semtimedop");
        }
    }

    return true;
}

SYS_NAMESPACE_END
