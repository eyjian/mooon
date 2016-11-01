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
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

INTEGER_ARG_DEFINE(int, lines, 10000, 1, 100000000, "number of lines");
INTEGER_ARG_DEFINE(uint32_t, size, 1024*1024*800, 1024, 1024*1024*2000, "size of a single log file");
INTEGER_ARG_DEFINE(uint16_t, backup, 10, 1, 100, "backup number of log file");
MOOON_NAMESPACE_USE

static void foo()
{
    for (int i=0; i<mooon::argument::lines->value(); ++i)
    {
        /* 1 */ MYLOG_DEBUG("[%d]MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n", i);
        /* 2 */ MYLOG_DETAIL("[%d]KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK\n", i+1);
        /* 3 */ MYLOG_INFO("[%d]BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n", i+2);
        /* 4 */ MYLOG_ERROR("[%d]TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n", i+3);
        /* 5 */ MYLOG_WARN("[%d]PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP\n", i+4);
        /* 6 */ MYLOG_TRACE("[%d]AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n", i+5);
        /* 7 */ MYLOG_STATE("[%d]ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\n", i+6);
        //sys::CUtils::millisleep(10);
    }
}

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }

    try
    {
        pid_t pid;
        ::mooon::sys::g_logger = new sys::CSafeLogger(".", "test.log");
        sys::g_logger->set_single_filesize(mooon::argument::size->value());
        sys::g_logger->set_backup_number(mooon::argument::backup->value());
        sys::g_logger->set_log_level(sys::LOG_LEVEL_DETAIL);
        sys::g_logger->enable_trace_log(true);

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

        MYLOG_INFO("hello\n");
        MYLOG_ERROR("%s\n", "world");

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

        int thread_lines = 7 * mooon::argument::lines->value();
        int all_threads_lines = 5 * thread_lines;
        int all_processes_lines = 10 * all_threads_lines;
        int total_lines = 2 + all_processes_lines;
        fprintf(stdout, "thread lines: %d\n", thread_lines);
        fprintf(stdout, "all threads lines: %d\n", all_threads_lines);
        fprintf(stdout, "all processes lines: %d\n", all_processes_lines);
        fprintf(stdout, "total lines: %d\n", total_lines);
    }
    catch (sys::CSyscallException& syscall_ex)
    {
        fprintf(stderr, "%s\n", syscall_ex.str().c_str());
        exit(1);
    }

    return 0;
}
