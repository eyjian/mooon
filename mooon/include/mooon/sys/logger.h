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
 *
 * CLogger和CLogThread间为多对一的关系
 *
 * CLogger的使用方法，四个步骤：
 * 1) logger* = new CLogger;
 * 2) logger->create(...);
 * 3) 使用，如logger->log_debug("%s.\n", "hello");
 * 4) 销毁：logger->destroy();，在这之后logger不能再被使用
 *
 * 请注意不需要delete logger;，否则将报编译错误
 */
#ifndef MOOON_SYS_LOGGER_H
#define MOOON_SYS_LOGGER_H
#include <mooon/sys/atomic.h>
#include <mooon/sys/event.h>
#include <mooon/sys/lock.h>
#include <mooon/sys/log.h>
#include <mooon/sys/thread.h>
#include <mooon/utils/array_queue.h>
#include <sys/epoll.h>
SYS_NAMESPACE_BEGIN

class CLogger;
class CLogThread;

/***
  * 常量定义
  */
enum
{
    LOGGER_NUMBER_MAX = 100,     /** 允许创建的最多Logger个数 */
    LOG_NUMBER_WRITED_ONCE = 10  /** 一次可连接写入的日志条数，最大不能超过IOV_MAX */
};

//////////////////////////////////////////////////////////////////////////
// log_message_t
typedef struct
{    
    uint16_t length; // 日志内容长度
    char content[4]; // 日志内容
}log_message_t;

//////////////////////////////////////////////////////////////////////////
// CLogProber
class CLogProber
{
public:
    CLogProber();
    virtual ~CLogProber();

    virtual bool execute() = 0;
    int get_fd() const { return _pipe_fd[0]; }

protected:
    void send_signal();
    void read_signal(int signal_number);

protected:
    int _pipe_fd[2];
};

//////////////////////////////////////////////////////////////////////////
// CLogger
class CLogger: public ILogger, public CLogProber, public CRefCountable
{
public:
    /** 构造一个Logger对象
      * @log_line_size: 默认情况下每行日志的最大长度，最长不会超过LOG_LINE_SIZE_MAX，否则截断
      */
    CLogger(uint16_t log_line_size=8192);

private:
    virtual ~CLogger();

public:
    /***
      * 销毁日志器，非线程安全，只有被一个线程调用一次
      * ，调用后，Logger不能再被使用。
      */
    void destroy();

    /** 日志器初始化，非线程安全，只有被一个线程调用一次
      * @log_path: 日志文件存放位置
      * @log_filename: 日志文件名，一包括路径部分
      * @log_queue_size: 所有日志队列加起来的总大小
      * @log_queue_number: 日志队列个数
      * @thread_orderly: 同一个线程的日志是否按时间顺序写
      * @exception: 如果出错抛出CSyscallException异常
      */
    void create(const char* log_path, const char* log_filename, uint32_t log_queue_size=1000);

    bool is_registered() const { return _registered; }
    void set_registered(bool registered) { _registered = registered; }

public:
    /** 是否允许同时在标准输出上打印日志 */
    virtual void enable_screen(bool enabled);
    /** 是否允许二进制日志，二进制日志必须通过它来打开 */
    virtual void enable_bin_log(bool enabled);
    /** 是否允许跟踪日志，跟踪日志必须通过它来打开 */
    virtual void enable_trace_log(bool enabled);
    /** 是否自动在一行后添加结尾的点号，如果最后已经有点号，则不会再添加 */
    virtual void enable_auto_adddot(bool enabled);
    /** 是否自动添加换行符，如果已经有换行符，则不会再自动添加换行符 */
    virtual void enable_auto_newline(bool enabled);    
    /** 设置日志级别，跟踪日志级别不能通过它来设置 */
    virtual void set_log_level(log_level_t log_level);
    /** 设置单个文件的最大建议大小 */
    virtual void set_single_filesize(uint32_t filesize);
    /** 设置备份日志的个数，如果为0，则不备份 */
    virtual void set_backup_number(uint16_t backup_number);

    virtual bool enabled_bin();

    virtual bool enabled_detail();
    virtual bool enabled_debug();
    virtual bool enabled_info();
    virtual bool enabled_warn();
    virtual bool enabled_error();
    virtual bool enabled_fatal();
    virtual bool enabled_state();
    virtual bool enabled_trace();

    virtual void log_detail(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_debug(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_info(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_warn(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_error(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_fatal(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_state(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));
    virtual void log_trace(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void log_bin(const char* filename, int lineno, const char* module_name, const char* log, uint16_t size);

private: // 日志文件操作
    void close_logfile();
    void create_logfile(bool truncate);
    bool need_rotate_file() const { return _current_bytes > (uint32_t)atomic_read(&_max_bytes); }
    void rotate_file();
    bool need_create_file() const;

private:
    virtual bool execute();
    void create_thread();
    bool prewrite();
    bool batch_write();
    bool single_write();
    void do_log(log_level_t log_level, const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    void push_log_message(log_message_t* log_message);
    
private:    
    int _log_fd;
    bool _auto_adddot;
    bool _auto_newline;
    uint16_t _log_line_size;
    atomic_t _log_level;
    bool _bin_log_enabled;
    bool _trace_log_enabled;

private:        
    bool _registered; // 是否已经被注册到LogThread中
    bool _destroying; // 正在被destroy
    bool _screen_enabled; 
    atomic_t _max_bytes;     
    atomic_t _backup_number;
    uint32_t _current_bytes;
    char _log_path[PATH_MAX];
    char _log_filename[FILENAME_MAX];
    utils::CArrayQueue<log_message_t*>* _log_queue;
    volatile int _waiter_number; // 等待PUSH消息的线程个数
    CEvent _queue_event;
    CLock _queue_lock; // 保护_log_queue的锁

private: // 所有Logger共享同一个CLogThread
    static CLock _thread_lock; // 保护_log_thread的锁
    static CLogThread* _log_thread;
};

//////////////////////////////////////////////////////////////////////////
// CLogThread
class CLogThread: public CThread, public CLogProber
{
public:
    CLogThread();
    ~CLogThread();

    void remove_logger(CLogger* logger);
    void register_logger(CLogger* logger);
    void inc_log_number() { atomic_inc(&_log_number); }
    void dec_log_number(int number) { atomic_sub(number, &_log_number); }
    int get_log_number() const { return atomic_read(&_log_number); }

private:
    virtual void run();
    virtual void before_stop() throw (utils::CException, CSyscallException);
    virtual void before_start() throw (utils::CException, CSyscallException);
    virtual bool execute();

private:
    void remove_object(CLogProber* log_prober);
    void register_object(CLogProber* log_prober);

private:    
    int _epoll_fd; 
    atomic_t _log_number; /** 队列中日志总条数 */
    struct epoll_event* _epoll_events;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_LOGGER_H
