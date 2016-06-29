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
#ifndef MOOON_SYS_FORK_SYNCHRONIZER_H
#define MOOON_SYS_FORK_SYNCHRONIZER_H
#include "mooon/sys/config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
SYS_NAMESPACE_BEGIN

// ForkSynchronizer解决的问题：
// 对于守护进程（daemon），不能简单的通过返回值来判断程序是否启动成功，
// 原因是启动逻辑可能发生在子进程，而父进程可能在子进程启动过程中就结束了。
// ForkSynchronizer提供一种机制，只有子进程启动成功后，父进程结束，
// 这样判断是否启动成功，就非常简单了，比如在bash中，可以通过“$?”来判断是否启动成功。
//
// 要做到这一点，借助ForkSynchronizer非常简单，
// 只需要父进程在主动结束前调用timed_wait_child_process_notification()，
// 同时子进程在启动成功后调用一下notify_parent_success()，
// 对notify_parent_success()的调用是必须的，不能省略，
// 但即使启动失败，也可不调用notify_parent_failure()。
class ForkSynchronizer
{
public:
    // CHILD_PROCESS_SUCCESS 子进程启动成功
    // CHILD_PROCESS_FAILURE 子进程启动失败
    // TIMEOUT 超时
    enum { CHILD_PROCESS_SUCCESS = 20150327, CHILD_PROCESS_FAILURE = 20121221, TIMEOUT = -1 };

public:
    ForkSynchronizer();
    ~ForkSynchronizer();

    // 只能由父进程调用
    // 当父进程调用时，会被阻塞，直到子进程调用了notify_parent_success()，或子进程任何形式的退出。
    // 约定子进程成功一定得调用notify_parent_success()，否则ForkSynchronizer无法正常工作。
    // child_pid 父进程fork()成功后的子进程的pid
    // milliseconds 等待子进程调用notify_parent_success()，或调用notify_parent_failure()，或子进程结束。
    // 返回值：如果调用成功则返回0，如果超时则返回-1，否则返回一个系统的errno
    int timed_wait_child_process_notification(pid_t child_pid, int milliseconds);
    
    // 只能由子进程调用
    // 用于通知父进程启动成功
    // 返回值：如果调用成功则返回0，否则返回一个系统的errno
    int notify_parent_success();
    
    // 只能由子进程调用
    // 用于通知父进程启动失败
    // 该函数可不主动调用
    // 返回值：如果调用成功则返回0，否则返回一个系统的errno
    int notify_parent_failure();

private:
    int notify_parent_process(int result);

private:
    int _pipe_fd[2];
};

extern int timed_poll(int fd, int events_requested, int milliseconds, int* events_returned);

ForkSynchronizer::ForkSynchronizer()
{
    _pipe_fd[0] = -1;
    _pipe_fd[1] = -1;

    if (pipe(_pipe_fd) != 0)
    {
    }
}

ForkSynchronizer::~ForkSynchronizer()
{
    if (_pipe_fd[1] != -1)
    {
        notify_parent_failure();
        close(_pipe_fd[1]);
    }

    if (_pipe_fd[0] != -1)
    {
        close(_pipe_fd[0]);
    }
}

int ForkSynchronizer::timed_wait_child_process_notification(pid_t child_pid, int milliseconds)
{
    if (_pipe_fd[1] != -1)
    {
        close(_pipe_fd[1]);
        _pipe_fd[1] = -1;
    }

    int result = 0;
    int ret = timed_poll(_pipe_fd[0], POLLIN, milliseconds, NULL); 

    if (-1 == ret)
    {
        return -1; // timeout
    }
    else if (0 == ret)
    {
        if (read(_pipe_fd[0], &result, sizeof(int)) != sizeof(int))
        {
            ret = errno;
        }
        else
        {
            if (CHILD_PROCESS_SUCCESS == result)
            {
                return 0;
            }

            int status = 0;
            waitpid(child_pid, &status, 0);
            return result; // CHILD_PROCESS_FAILURE
        }
    }
    else
    {
        return ret;
    }
}

int ForkSynchronizer::notify_parent_success()
{
    return notify_parent_process(CHILD_PROCESS_SUCCESS);
}

int ForkSynchronizer::notify_parent_failure()
{
    return notify_parent_process(CHILD_PROCESS_FAILURE);
}

int ForkSynchronizer::notify_parent_process(int result)
{
    int ret = 0;

    if (write(_pipe_fd[1], &result, sizeof(int)) != sizeof(int))
    {
        ret = errno;
    }

    // close(_pipe_fd[0]);
    // close(_pipe_fd[1]);
    return ret;
}

int timed_poll(int fd, int events_requested, int milliseconds, int* events_returned)
{
    int remaining_milliseconds = milliseconds;
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = events_requested; // POLLIN | POLLOUT | POLLERR;

    for (;;)
    {        
        time_t begin_seconds = time(NULL);
        int retval = poll(fds, sizeof(fds)/sizeof(struct pollfd), remaining_milliseconds);
        if (retval > 0)
            break;

        // 超时返回false
        if (0 == retval)
            return -1; // timeout
        
        // 出错抛出异常
        if (-1 == retval)
        {
            // 中断，则继续
            if (EINTR == errno)
            {
                // 保证时间总是递减的，虽然会引入不精确问题，但总是好些，极端情况下也不会死循环
                time_t gone_milliseconds = (time(NULL)-begin_seconds) * 1000 + 10;
                remaining_milliseconds = (remaining_milliseconds > gone_milliseconds)? remaining_milliseconds - gone_milliseconds: 0;                
                continue;
            }

            return errno;
        }
    }

    if (events_returned != NULL) *events_returned = fds[0].revents;
    return 0;
}

/*
#include "fork_synchronizer.h"

static void test(bool suceessful);

extern int main()
{
    printf("[P]=> test success:\n");
    test(true);

    printf("\n");
    printf("[P]=> test failure:\n");
    test(false);

    return 0;
}

void test(bool successful)
{
    ForkSynchronizer fs;

    pid_t pid = fork();
    if (-1 == pid)
    {
        printf("[P]fork error: %m\n");
    }
    else if (pid > 0)
    {
        printf("[P]parent process[%d] to wait child process notification\n", getpid());

        while (true)
        {
           int milliseconds = 2000;
            int result = fs.timed_wait_child_process_notification(pid, milliseconds);
            if (0 == result)
            {
                printf("[P]child process[%d] start successfully\n", pid);
                break;
            }
            else if (-1 == result)
            {
                printf("[P]continue to wait child process[%d] notification ...\n", pid);
            }
            else
            {
                fprintf(stderr, "[P]child process[%d] start failed: %s\n", pid, strerror(result));
                break;
            }
        }
    }
    else
    {
        unsigned int seconds = 6;
        printf("[C]child process: %d:%d to sleep %d seconds\n", getpid(), getppid(), seconds);

        sleep(seconds);
        if (successful)
        {
            if (fs.notify_parent_success() != 0)
            {
                fprintf(stderr, "[C]failed to notify parent success\n");
            }
            else
            {
                printf("[C]child process: %d:%d to sleep %d seconds again\n", getpid(), getppid(), seconds+seconds);
                sleep(seconds+seconds);
            }
        }

        printf("[C]child process[%d:%d] exited\n", getpid(), getppid());
        exit(0);
    }
}
*/
SYS_NAMESPACE_END
#endif // MOOON_SYS_FORK_SYNCHRONIZER_H
