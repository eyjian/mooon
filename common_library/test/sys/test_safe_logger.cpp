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
 * Author: eyjian@qq.com or eyjian@gmail.com or eyjian@live.com
 */
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
MOOON_NAMESPACE_USE

static void foo()
{
    for (int i=0; i<1000; ++i)
    {
        MYLOG_INFO("foo => %d", i);
        MYLOG_ERROR("foo => %d", i+1);

        sys::CUtils::millisleep(10);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        pid_t pid;
        ::mooon::sys::g_logger = new sys::CSafeLogger(".", "test.log");

        MYLOG_INFO("hello");
        MYLOG_ERROR("world");

        for (int i=0; i<10; ++i)
        {
            pid = fork();
            if (-1 == pid)
            {
                fprintf(stderr, "fork error: %m\n");
                exit(1);
            }
            else if (0 == pid)
            {
                // 子进程
                sys::CThreadEngine thread1(sys::bind(&foo));
                sys::CThreadEngine thread2(sys::bind(&foo));
                sys::CThreadEngine thread3(sys::bind(&foo));
                sys::CThreadEngine thread4(sys::bind(&foo));
                sys::CThreadEngine thread5(sys::bind(&foo));

                thread1.join();
                thread2.join();
                thread3.join();
                thread4.join();
                thread5.join();
                exit(0);
            }
        }

        // 等待所有子进程结束
        while (true)
        {
            int status = -1;
            pid = wait(&status);
            if (pid > 0)
            {
                printf("process[%u] exit with %d\n", pid, status);
            }
            else
            {
                if (ECHILD == errno)
                {
                    break;
                }
                else
                {
                    printf("unknown error when waiting child\n");
                    break;
                }
            }
        }
    }
    catch (sys::CSyscallException& syscall_ex)
    {
        fprintf(stderr, "%s\n", syscall_ex.str().c_str());
        exit(1);
    }

    return 0;
}
