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
#include <mooon/sys/log.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/utils/args_parser.h>
#include <signal.h>

// 指定ReportSelf的配置文件（值为空时表示不上报），
// 如果report_self_conf指定的文件不存在或配置错误等，将均不能上报
// 默认值为：/etc/mooon_report_self.conf
//
// report_self_conf文件格式示例（JSON格式）：
// {
//     "ethX":"eth0",
//     "report_self_servers":"127.0.0.1:7110,127.0.0.2:7110"
// }
STRING_ARG_DECLARE(report_self_conf);

// ReportSelf上报间隔（单位：秒，值为0时表示不上报）
// 默认值为3600，即一小时上报一次
INTEGER_ARG_DECLARE(int, report_self_interval);

SYS_NAMESPACE_BEGIN

// 是否收到了退出信号，值为大于0，表示收到了退出信号
extern int exit_by_signal;

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

// CMainHelper内置了优雅退出
//
// 使用示例：
// class CMyMainHelper: public mooon::sys::CMainHelper
// {
// private:
//     virtual bool on_init(int argc, char* argv[])
//     {
//         library_init(argc, argv);
//         return true;
//     }
//
//     virtual bool on_run()
//     {
//         return _myserver.start();
//     }
//
//     virtual void on_fini()
//     {
//         mooon::sys::CMySQLConnection::library_end();
//     }
//
//     virtual void on_terminated()
//     {
//         // 一定要最先调用父类的on_terminated
//         mooon::sys::CMainHelper::on_terminated();
//         // 停止CMyServer
//         _myserver.stop();
//     }
//
// private:
//     CMyServer _myserver;
// };
class CMainHelper: public IMainHelper
{
public:
    // log_level_signo
    // 动态调整日志级别的信号，通过该信号可让日志级别在DEBUG和INFO两个级别间互相切换
    // 如果值为0或负值，表示不启用该功能
    CMainHelper(int log_level_signo=SIGUSR2);
    ~CMainHelper();
    void signal_thread();

private:
    // 子类一般不要重写init，
    // init()过程依次为：
    // 1) 命令行参数解析
    // 2) 调用子类的on_check_parameter()做参数检查
    // 3) 创建SafeLogger，之后可用MYLOG_xxx记录日志
    // 4) 阻塞信号SIGTERM
    // 5) 调用子类的on_block_signal()
    // 6) 调用子类的on_init()
    // 7) 创建信号线程signal_thread
    //
    // 并捕获了CSyscallException和Exception两个异常
    virtual bool init(int argc, char* argv[]);
    virtual bool run() { return on_run(); }
    virtual void fini();

private:
    // 参数检查
    virtual bool on_check_parameter() { return true; }

    // CMainHelper内置阻塞了信号SIGTERM，
    // 如果需要，子类可以在on_block_signal中阻塞其它信号，信号发生时，on_signal_handler被调用
    virtual void on_block_signal() { /* mooon::sys::CSignalHandler::block_signal(SIGUSR1); */ }

    // on_init需子类重写
    virtual bool on_init(int argc, char* argv[]) = 0;

    // 让run和init等统一
    virtual bool on_run() { return true; }

    // 子类可选择是否重写on_fini
    // 这个时候信号线程已经退出
    virtual void on_fini() {}

public: // 信号相关的
    // 特别注意：
    // 子类可重写on_terminated，
    // 但在重写前一定要先调用CMainHelper::on_terminated()，
    // 在CMainHelper::on_terminated()中会置_stop为true，这是优雅退出的前提
    virtual void on_terminated();
    virtual void on_child_end(pid_t child_pid, int child_exited_status);
    virtual void on_exception(int errcode);

    // 非SIGTERM信号发生时被调用
    //
    // 注意：
    // 如果子类重写on_signal_handler，则应首先调用CMainHelper::on_signal_handler，
    // 否则动态日志级别调整功能将失效。
    virtual void on_signal_handler(int signo);

protected:
    bool to_stop() const { return _stop; }

private:
    volatile bool _stop;
    int _log_level_signo;
    mooon::sys::CThreadEngine* _signal_thread;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_MAIN_TEMPLATE_H
