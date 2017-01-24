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
 * Author: jian yi, eyjian@qq.com
 */
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <stdexcept>
#include <sys/wait.h>
#include <utils/string_utils.h>
#include "sys/log.h"
#include "sys/utils.h"
#include "sys/main_template.h"
SYS_NAMESPACE_BEGIN

/***
  * 是否自重启，下列信号发生时，进程是否自重启:
  * 1) SIGILL
  * 2) SIGFPE
  * 3) SIGBUS
  * 4) SIGABRT
  * 5) SIGSEGV
  */
static bool self_restart(IMainHelper* main_helper);

/***
  * 子进程处理逻辑
  */
static void child_process(IMainHelper* main_helper, int argc, char* argv[]);

/***
  * 父进程处理逻辑
  * @child_pid: 子进程号
  * @child_exit_code: 子进程的退出代码
  * @return: 返回true的情况下才会自重启，否则父子进程都退出
  */
static bool parent_process(IMainHelper* main_helper, pid_t child_pid, int& child_exit_code);

// 是否收到了退出信号
int exit_by_signal = 0;

// 信号线程
static sigset_t sg_sigset;
static pthread_t sg_signal_thread;
static void* signal_thread_proc(void* param)
{
    IMainHelper* main_helper = static_cast<IMainHelper*>(param);
    int exit_signo = main_helper->get_exit_signal();

    while (true)
    {
        int signo = -1;
        int errcode = sigwait(&sg_sigset, &signo);
        if (EINTR == errcode)
        {
            continue;
        }
        if (errcode != 0)
        {
            __MYLOG_ERROR(main_helper->get_logger(), NULL, "Waited signal error: %s.\n", sys::Error::to_string().c_str());
            break;
        }
        if (exit_signo == signo)
        {
            exit_by_signal = signo; // 标记
            __MYLOG_INFO(main_helper->get_logger(), NULL, "Received exit signal %s and exited.\n", strsignal(signo));
            break;
        }
    }

    return NULL;
}

/***
  * main_template总是在main函数中调用，通常如下一行代码即可:
  * int main(int argc, char* argv[])
  * {
  *     return main_template(argc, argv);
  * }
  */
int main_template(IMainHelper* main_helper, int argc, char* argv[])
{
    // 退出代码，由子进程决定
    int exit_code = 1;

    // 忽略掉PIPE信号
    if (main_helper->ignore_pipe_signal())
    {
        if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
        {
            fprintf(stderr, "Ignored SIGPIPE error: %s.\n", sys::CUtils::get_last_error_message().c_str());
            return 1;
        }
    }

    while (true)
    {
        pid_t pid = self_restart(main_helper)? fork(): 0;
        if (-1 == pid)
        {
            // fork失败
            fprintf(stderr, "fork error: %s.\n", sys::CUtils::get_last_error_message().c_str());
            break;
        }
        else if (0 == pid)
        {
            child_process(main_helper, argc, argv);
        }
        else if (!parent_process(main_helper, pid, exit_code))
        {
            break;
        }
    }

    return exit_code;
}

bool self_restart(IMainHelper* main_helper)
{
    std::string env_name = main_helper->get_restart_env_name();
    utils::CStringUtils::trim(env_name);

    // 如果环境变量名为空，则认为总是自重启
    if (env_name.empty()) return true;

    // 由环境变量SELF_RESTART来决定是否自重启
    char* restart = getenv(env_name.c_str());
    return (restart != NULL)
        && (0 == strcasecmp(restart, "true"));
}

void child_process(IMainHelper* main_helper, int argc, char* argv[])
{
    int errcode = 0;
    //sigset_t sigset;
    sigset_t old_sigset;
    
    int exit_signo = main_helper->get_exit_signal();
    if (exit_signo > 0)
    {            
        // 收到SIGUSR1信号时，则退出进程
        if (-1 == sigemptyset(&sg_sigset))
        {
            fprintf(stderr, "Initialized signal set error: %s.\n", sys::CUtils::get_last_error_message().c_str());
            exit(1);
        }
        if (-1 == sigaddset(&sg_sigset, exit_signo))
        {
            fprintf(stderr, "Added %s to signal set error: %s.\n", strsignal(exit_signo), sys::CUtils::get_last_error_message().c_str());
            exit(1);
        }
        if (-1 == sigprocmask(SIG_BLOCK, &sg_sigset, &old_sigset))
        {
            fprintf(stderr, "Blocked SIGUSR1 error: %s\n", sys::CUtils::get_last_error_message().c_str());
            exit(1);
        }

        // 创建信号线程
        errcode = pthread_create(&sg_signal_thread, NULL, signal_thread_proc, main_helper);
        if (errcode != 0)
        {
            fprintf(stderr, "Create signal thread failed: %s\n", sys::CUtils::get_error_message(errcode).c_str());
            exit(1);
        }
    }
    
    // 设置环境变量TZ，以优化localtime、localtime_r和mktime的性能
    if (NULL == getenv("TZ"))
    {
        //setenv("TZ", "", 1);
        setenv("TZ", "Asia/Shanghai", 1);
    }

    // 请注意：只有在init成功后，才可以使用__MYLOG_INFO写日志，否则这个时候日志器可能还未created出来
    if (!main_helper->init(argc, argv))
    {
        //fprintf(stderr, "Main helper initialized failed.\n");
		main_helper->fini();
        exit(1);
    }
    if (!main_helper->run())
	{
		//fprintf(stderr, "Main helper run failed.\n");
		main_helper->fini();
		exit(1);
	}

	// 记录用来退出的信号
	//__MYLOG_INFO(main_helper->get_logger(), "Exit signal is %s .\n", strsignal(exit_signo));
	
	// 记录工作进程号
    //__MYLOG_INFO(main_helper->get_logger(), "Work process is %d.\n", sys::CUtils::get_current_process_id());

    // 等待信号线程退出
    if (exit_signo > 0)
    {
        errcode = pthread_join(sg_signal_thread, NULL);
        if (errcode != 0)
        {
            fprintf(stderr, "Join signal thread failed: %s\n", sys::CUtils::get_error_message(errcode).c_str());
        }
    }

    main_helper->fini();
    exit(errcode);
}

bool parent_process(IMainHelper* main_helper, pid_t child_pid, int& child_exit_code)
{
    // 是否重启动
    bool restart = false;
    fprintf(stdout, "Parent process is %d, and its work process is %d.\n", sys::CUtils::get_current_process_id(), child_pid);

    while (true)
    {
        int status;
        int retval = waitpid(child_pid, &status, 0);
        if (-1 == retval)
        {
            if (EINTR == errno)
            {
                continue;
            }
            else
            {
                fprintf(stderr, "Wait %d error: %s.\n", child_pid, sys::CUtils::get_last_error_message().c_str());
            }
        }
        else if (WIFSTOPPED(status))
        {
            child_exit_code = WSTOPSIG(status);
            fprintf(stderr, "Process %d was stopped by signal %d.\n", child_pid, child_exit_code);
        }
        else if (WIFEXITED(status))
        {
            child_exit_code = WEXITSTATUS(status);
            fprintf(stderr, "Process %d was exited with code %d.\n", child_pid, child_exit_code);
        }
        else if (WIFSIGNALED(status))
        {                    
            int signo = WTERMSIG(status);
            fprintf(stderr, "Process %d received signal %s.\n", child_pid, strsignal(signo));
            child_exit_code = signo;

            if ((SIGILL == signo)   // 非法指令
             || (SIGBUS == signo)   // 总线错误
             || (SIGFPE == signo)   // 浮点错误
             || (SIGSEGV == signo)  // 段错误
             || (SIGABRT == signo)) // raise
            {
                restart = true;
                fprintf(stderr, "Process %d will restart self for signal %s.\n", child_pid, strsignal(signo));

                // 延迟一秒，避免极端情况下拉起即coredump带来的死循环问题
                sys::CUtils::millisleep(main_helper->get_restart_milliseconds());
            }
        }
        else
        {
            fprintf(stderr, "Process %d was exited, but unknown error.\n", child_pid);
        }

        break;
    }

    return restart;
}

SYS_NAMESPACE_END
