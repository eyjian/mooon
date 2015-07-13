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
 * 主要功能:
 * 1) 自动重启进程功能，要求环境变量SELF_RESTART存在，且值为true，其中true不区分大小写
 * 2) 初始化init和反初始化fini函数的自动调用，注意如果初始化init不成功，则不会调用反初始化fini
 * 3) 收到SIGUSR1信号退出进程，退出之前会调用fini
 * 注意，只支持下列信号发生时的自动重启:
 * SIGILL，SIGBUS，SIGFPE，SIGSEGV，SIGABRT
 */
#ifndef MOOON_SYS_MAIN_TEMPLATE_H
#define MOOON_SYS_MAIN_TEMPLATE_H
#include "mooon/sys/log.h"
#include <signal.h>
SYS_NAMESPACE_BEGIN

/***
  * main函数辅助接口，用于帮助自定义的初始化
  */
class IMainHelper
{
public:
    virtual ~IMainHelper() {}

    /***
      * 初始化，进程开始时调用
	  * 不管init返回true还是false，在进程退出之前一步都是调用fini
	  * 如果init返回false，则fini会被调用，并退出进程，退出码为1
      */
    virtual bool init(int argc, char* argv[]) = 0;

    /***
      * 执行过程
      * 如果返回false，则会fini会被调用，并退出进程，退出码为1
      */
    virtual bool run() { return true; }

    /***
      * 反初化，进程退出之前调用
      */
    virtual void fini() = 0;

    /***
      * 得到日志器
      * @return 如果返回NULL则直接输出到屏幕
      */
    virtual ILogger* get_logger() const { return NULL; }

    /***
      * 得到退出信号，即收到该信号时，进程自动退出，退出之前调用fini
      * @return 如果返回0，表示不做任何处理，否则收到指定信号时退出
      */
    virtual int get_exit_signal() const { return 0; }

    /***
      * 是否忽略PIPE信号
      * @return 如果返回true，表示忽略PIPE信号，否则不忽略
      */
    virtual bool ignore_pipe_signal() const { return true; }

    /***
      * 得到控制重启功能的环境变量名
      * @return 如果返回空，包括空格，则表示总是自重启，
      *  否则是否自重启，由同名的环境变量值决定，值为true则自重启，否则不自重启
      */
    virtual std::string get_restart_env_name() const { return "SELF_RESTART"; }
    
    /***
      * 得到重启间隔微秒数，默认为1秒
      */
    virtual uint32_t get_restart_milliseconds() const { return 1000; }
};

/***
  * 通用main函数的模板，
  * main_template总是在main函数中调用，通常如下一行代码即可:
  * int main(int argc, char* argv[])
  * {
  *     IMainHelper* main_helper = new CMainHelper();
  *     return main_template(main_helper, argc, argv);
  * }
  */
extern int main_template(IMainHelper* main_helper, int argc, char* argv[]);

SYS_NAMESPACE_END
#endif // MOOON_SYS_MAIN_TEMPLATE_H
