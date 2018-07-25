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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com or eyjian@live.com
 */
#ifndef MOOON_SYS_SAFE_LOGGER_H
#define MOOON_SYS_SAFE_LOGGER_H
#include <mooon/sys/log.h>
#include <mooon/sys/atomic.h>
#include <mooon/sys/read_write_lock.h>
#include <mooon/sys/syscall_exception.h>
#include <stdio.h>
SYS_NAMESPACE_BEGIN

// CSafeLogger支持：
// 1) 通过环境变量名MOOON_LOG_LEVEL来控制日志级别
// 2) 通过环境变量名MOOON_LOG_SCREEN来控制是否在屏幕上输出日志
// 3) 通过环境变量名MOOON_LOG_TRACE来控制是否显示trace日志
// 4) 通过环境变量名MOOON_LOG_FILESIZE来控制单个日志文件的大小
// 5) 通过环境变量名MOOON_LOG_BACKUP来控制日志文件备份个数
class CSafeLogger;

// 根据程序文件创建CSafeLogger
//
// 假设目录结构为：
// home
// |-- bin
// |-- log
//
// 可执行程序文件放在bin子目录，则日志文件将自动放在log子目录下，
// 如果enable_program_path为true，则log子目录不存在时，日志文件将放在和可执行程序文件同一目录下。
//
// suffix 日志文件名后缀
// 假设程序名为test，后缀为空则日志文件名为test.log，如果后缀为6789则日志文件名为test_6789.log
//
// 若因目录和文件名，或者创建、打开文件权限等问题，则会抛出CSyscallException异常
extern CSafeLogger* create_safe_logger(bool enable_program_path=true, uint16_t log_line_size=SIZE_8K, const std::string& suffix=std::string(""), bool enable_syslog=false) throw (CSyscallException);

// 根据程序文件创建CSafeLogger
// 若因目录和文件名，或者创建、打开文件权限等问题，则会抛出CSyscallException异常
//
// 适用于CGI记录日志，原因是CGI不能使用程序文件名，
// 1) 假设CGI的cpp文件名为mooon.cpp，则日志文件名为mooon.log
// 2) 假设CGI的cpp文件名为mooon.cc，则日志文件名为mooon.log
// 使用示例：
// mooon::sys::g_logger = create_safe_logger(logdir, __FILE__);
extern CSafeLogger* create_safe_logger(const std::string& log_dirpath, const std::string& cpp_filename, uint16_t log_line_size=8192, bool enable_syslog=false) throw (CSyscallException);

/**
  * 多线程和多进程安全的日志器
  */
class CSafeLogger: public ILogger
{
public:
    CSafeLogger(const char* log_dir, const char* log_filename, uint16_t log_line_size=8192, bool enable_syslog=false) throw (CSyscallException);
    virtual ~CSafeLogger();

    virtual int get_log_level() const;
    virtual std::string get_log_dir() const;
    virtual std::string get_log_filename() const;
    virtual std::string get_log_filepath() const;
    virtual std::string get_log_shortname() const;

    /** 是否允许同时在标准输出上打印日志 */
    virtual void enable_screen(bool enabled);
    /** 是否允许二进制日志，二进制日志必须通过它来打开 */
    virtual void enable_bin_log(bool enabled);
    /** 是否允许跟踪日志，跟踪日志必须通过它来打开 */
    virtual void enable_trace_log(bool enabled);
    /** 是否允许裸日志，裸日志必须通过它来打开 */
    virtual void enable_raw_log(bool enabled, bool record_time=false);
    /** 是否自动在一行后添加结尾的点号，如果最后已经有点号或换符符，则不会再添加 */
    virtual void enable_auto_adddot(bool enabled);
    /** 是否自动添加换行符，如果已经有换行符，则不会再自动添加换行符 */
    virtual void enable_auto_newline(bool enabled);
    /** 设置日志级别，跟踪日志级别不能通过它来设置 */
    virtual void set_log_level(log_level_t log_level);
    /** 设置单个文件的最大建议大小 */
    virtual void set_single_filesize(uint32_t filesize);
    /** 设置日志文件备份个数，不包正在写的日志文件 */
    virtual void set_backup_number(uint16_t backup_number);

    /** 是否允许二进制日志 */
    virtual bool enabled_bin();
    /** 是否允许Detail级别日志 */
    virtual bool enabled_detail();
    /** 是否允许Debug级别日志 */
    virtual bool enabled_debug();
    /** 是否允许Info级别日志 */
    virtual bool enabled_info();
    /** 是否允许Warn级别日志 */
    virtual bool enabled_warn();
    /** 是否允许Error级别日志 */
    virtual bool enabled_error();
    /** 是否允许Fatal级别日志 */
    virtual bool enabled_fatal();
    /** 是否允许输出状态日志 */
    virtual bool enabled_state();
    /** 是否允许Trace级别日志 */
    virtual bool enabled_trace();
    /** 是否允许Raw级别日志 */
    virtual bool enabled_raw();

    virtual void vlog_detail(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_detail(const char* filename, int lineno, const char* module_name, const char* format, ...)  __attribute__((format(printf, 5, 6)));

    virtual void vlog_debug(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_debug(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void vlog_info(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_info(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void vlog_warn(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_warn(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void vlog_error(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_error(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void vlog_fatal(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_fatal(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void vlog_state(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_state(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    virtual void vlog_trace(const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    virtual void log_trace(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)));

    /** 写裸日志 */
    virtual void vlog_raw(const char* format, va_list& args);
    virtual void log_raw(const char* format, ...) __attribute__((format(printf, 2, 3)));

    /** 写二进制日志 */
    virtual void log_bin(const char* filename, int lineno, const char* module_name, const char* log, uint16_t size);

private:
    bool need_rotate(int fd) const;
    void do_log(log_level_t log_level, const char* filename, int lineno, const char* module_name, const char* format, va_list& args);
    void rotate_log();
    void write_log(const char* log_line, int log_line_size);

private:
    int prepare_log_fd();

private:
    CReadWriteLock _read_write_lock;
    int _log_fd;

private:
    bool _auto_adddot;
    bool _auto_newline;
    uint16_t _log_line_size;
    atomic_t _log_level;
    bool _sys_log_enabled;
    bool _bin_log_enabled;
    bool _trace_log_enabled;
    bool _raw_log_enabled;
    bool _raw_record_time;

private:
    bool _screen_enabled;
    atomic_t _max_bytes;
    atomic_t _backup_number;
    const std::string _log_dir;
    const std::string _log_filename;
    const std::string _log_filepath;
    const std::string _log_shortname;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SAFE_LOGGER_H
