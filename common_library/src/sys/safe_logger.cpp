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
#include "mooon/sys/safe_logger.h"
#include "mooon/sys/datetime_utils.h"
#include "mooon/sys/file_locker.h"
#include "mooon/sys/file_utils.h"
#include "mooon/utils/scoped_ptr.h"
#include "mooon/utils/string_utils.h"
#include <pthread.h>
#include <sstream>
#include <unistd.h>
SYS_NAMESPACE_BEGIN

// 线程级别的
static __thread int sg_thread_log_fd = -1;

void close_thread_log_fd()
{
    if (sg_thread_log_fd != -1)
    {
        if (-1 == close(sg_thread_log_fd))
            fprintf(stderr, "[%d:%lu] SafeLogger close error: %m\n", getpid(), pthread_self());

        sg_thread_log_fd = -1;
    }
}

CSafeLogger* create_safe_logger(bool enable_program_path, uint16_t log_line_size) throw (CSyscallException)
{
    std::string log_dirpath = get_log_dirpath(enable_program_path);
    std::string log_filename = get_log_filename();
    CSafeLogger* logger = new CSafeLogger(log_dirpath.c_str(), log_filename.c_str(), log_line_size);

    set_log_level_by_env(logger);
    return logger;
}

CSafeLogger* create_safe_logger(const std::string& log_dirpath, const std::string& cpp_filename, uint16_t log_line_size) throw (CSyscallException)
{
    std::string log_filename = utils::CStringUtils::replace_suffix(cpp_filename, ".log");
    CSafeLogger* logger = new CSafeLogger(log_dirpath.c_str(), log_filename.c_str(), log_line_size);

    set_log_level_by_env(logger);
    return logger;
}

////////////////////////////////////////////////////////////////////////////////
CSafeLogger::CSafeLogger(const char* log_dir, const char* log_filename, uint16_t log_line_size) throw (CSyscallException)
    :_auto_adddot(false)
    ,_auto_newline(true)
    ,_bin_log_enabled(false)
    ,_trace_log_enabled(false)
    ,_screen_enabled(false)
    ,_log_dir(log_dir)
    ,_log_filename(log_filename)
{
    atomic_set(&_max_bytes, DEFAULT_LOG_FILE_SIZE);
    atomic_set(&_log_level, LOG_LEVEL_INFO);
    atomic_set(&_backup_number, DEFAULT_LOG_FILE_BACKUP_NUMBER);

    // 保证日志行最大长度不小于指定值
    _log_line_size = (log_line_size < LOG_LINE_SIZE_MIN)? LOG_LINE_SIZE_MIN: log_line_size;
    if (_log_line_size > LOG_LINE_SIZE_MAX)
        _log_line_size = LOG_LINE_SIZE_MAX;

    _log_filepath = _log_dir + std::string("/") + _log_filename;
    _log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);
    if (-1 == _log_fd)
    {
        int errcode = errno;
        fprintf(stderr, "[%d:%lu] SafeLogger open %s error: %m\n", getpid(), pthread_self(), _log_filepath.c_str());

        THROW_SYSCALL_EXCEPTION(NULL, errcode, "open");
    }
}

CSafeLogger::~CSafeLogger()
{
    if (_log_fd != -1)
    {
        close(_log_fd);
    }
}

void CSafeLogger::enable_screen(bool enabled)
{
    _screen_enabled = enabled;
}

void CSafeLogger::enable_bin_log(bool enabled)
{
    _bin_log_enabled = enabled;
}

void CSafeLogger::enable_trace_log(bool enabled)
{
    _trace_log_enabled = enabled;
}

void CSafeLogger::enable_auto_adddot(bool enabled)
{
    _auto_adddot = enabled;
}

void CSafeLogger::enable_auto_newline(bool enabled)
{
    _auto_newline = enabled;
}

void CSafeLogger::set_log_level(log_level_t log_level)
{
    atomic_set(&_log_level, log_level);
}

void CSafeLogger::set_single_filesize(uint32_t filesize)
{
    uint32_t max_bytes = (filesize < LOG_LINE_SIZE_MIN*10)? LOG_LINE_SIZE_MIN*10: filesize;
    atomic_set(&_max_bytes, max_bytes);
}

void CSafeLogger::set_backup_number(uint16_t backup_number)
{
    atomic_set(&_backup_number, backup_number);
}

bool CSafeLogger::enabled_bin()
{
    return _bin_log_enabled;
}

bool CSafeLogger::enabled_detail()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_DETAIL;
}

bool CSafeLogger::enabled_debug()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_DEBUG;
}

bool CSafeLogger::enabled_info()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_INFO;
}

bool CSafeLogger::enabled_warn()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_WARN;
}

bool CSafeLogger::enabled_error()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_ERROR;
}

bool CSafeLogger::enabled_fatal()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_FATAL;
}

bool CSafeLogger::enabled_state()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_STATE;
}

bool CSafeLogger::enabled_trace()
{
    return _trace_log_enabled;
}

void CSafeLogger::log_detail(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_detail())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_DETAIL, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_debug(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_debug())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_DEBUG, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_info(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_info())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_INFO, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_warn(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_warn())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_WARN, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_error(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_error())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_ERROR, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_fatal(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_fatal())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_FATAL, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_state(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_state())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_STATE, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::log_trace(const char* filename, int lineno, const char* module_name, const char* format, ...)
{
    if (enabled_trace())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_TRACE, filename, lineno, module_name, format, args);
    }
}

void CSafeLogger::bin_log(const char* filename, int lineno, const char* module_name, const char* log, uint16_t size)
{
    if (enabled_bin())
    {
        //set_log_length(log, size);
        //_log_thread->push_log(log);
    }
}

int CSafeLogger::get_thread_log_fd() const
{
    if (sg_thread_log_fd != _log_fd)
    {
        if (sg_thread_log_fd != -1)
            close(sg_thread_log_fd);

        LockHelper<CLock> lock_helper(_lock);
        if (-1 == _log_fd)
        {
            sg_thread_log_fd = -1;
        }
        else
        {
            sg_thread_log_fd = dup(_log_fd);
        }
    }

    return sg_thread_log_fd;
}

bool CSafeLogger::need_rotate(int fd) const
{
    return CFileUtils::get_file_size(fd) > static_cast<off_t>(_max_bytes);
}

void CSafeLogger::do_log(log_level_t log_level, const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    std::stringstream log_header; // 每条日志的头
    char datetime[sizeof("2012-12-12 12:12:12")];
    CDatetimeUtils::get_current_datetime(datetime, sizeof(datetime));

    // 日志头内容：[日期][线程ID/进程ID][日志级别][模块名][代码文件名][代码行号]
    log_header << "[" << datetime << "]"
               << "[" << pthread_self() << "/" << getpid() << "]"
               << "[" << get_log_level_name(log_level) << "]";
    if (module_name != NULL)
        log_header << "[" << module_name << "]";
    log_header << "[" << filename << ":" << lineno << "]";

    int m, n, log_real_size;
    utils::ScopedArray<char> log_line(new char[_log_line_size]);
    m = snprintf(log_line.get(), _log_line_size, "%s", log_header.str().c_str());
    n = vsnprintf(log_line.get()+m, _log_line_size-m, format, args);
    log_real_size = m + n;

    // 是否自动添加结尾用的点号
    if (_auto_adddot)
    {
        // 如果已有结尾的点，则不再添加，以免重复
        if (log_line.get()[log_real_size-1] != '.')
        {
            log_line.get()[log_real_size] = '.';
            ++log_real_size;
        }
    }

    // 是否自动换行
    if (_auto_newline)
    {
        // 如果已有一个换行符，则不再添加
        if (log_line.get()[log_real_size-1] != '\n')
        {
            log_line.get()[log_real_size] = '\n';
            ++log_real_size;
        }
    }

    // 允许打屏
    if (_screen_enabled)
    {
        (void)write(STDOUT_FILENO, log_line.get(), log_real_size);
    }

    // 写日志文件
    int thread_log_fd = get_thread_log_fd();
    if (thread_log_fd != -1)
    {
        int bytes = write(thread_log_fd, log_line.get(), log_real_size);
        if (-1 == bytes)
        {
            fprintf(stderr, "[%d:%lu] SafeLogger[%d] write error: %m\n", getpid(), pthread_self(), thread_log_fd);
        }
        else if (bytes > 0)
        {
            try
            {
                // 判断是否需要滚动
                if (need_rotate(thread_log_fd))
                {
                    std::string lock_path = _log_dir + std::string("/.") + _log_filename + std::string(".lock");
                    FileLocker file_locker(lock_path.c_str(), true); // 确保这里一定加锁

                    // _fd可能已被其它进程或线程滚动了，所以这里需要重新open一下
                    int log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);
                    if (-1 == log_fd)
                    {
                        fprintf(stderr, "[%d:%lu] SafeLogger open %s error: %m\n", getpid(), pthread_self(), _log_filepath.c_str());
                    }
                    else
                    {
                        try
                        {
                            if (need_rotate(log_fd))
                            {
                                close(log_fd);
                                rotate_log();
                            }
                            else
                            {
                                // 如果是线程执行了滚动，则_log_fd值为非-1，可直接使用
                                // 如果是其它进程执行了滚动了，则应当使用log_fd替代_log_fd

                                // 可以放在锁FileLocker之外，用来保护_log_fd
                                // 由于每个线程并不直接使用_log_fd，而是对_log_fd做了dup，所以只要保护_log_fd即可
                                LockHelper<CLock> lock_helper(_lock);

                                close(_log_fd);
                                _log_fd = log_fd;
                            }
                        }
                        catch (CSyscallException& syscall_ex)
                        {
                            fprintf(stderr, "[%d:%lu] SafeLogger[%d] rotate error: %s.\n", getpid(), pthread_self(), log_fd, syscall_ex.str().c_str());
                        }
                    }
                }
            }
            catch (CSyscallException& syscall_ex)
            {
                fprintf(stderr, "[%d:%lu] SafeLogger[%d] rotate error: %s\n", getpid(), pthread_self(), thread_log_fd, syscall_ex.str().c_str());
            }
        }
    }
}

void CSafeLogger::rotate_log()
{
    std::string new_path;  // 滚动后的文件路径，包含目录和文件名
    std::string old_path;  // 滚动前的文件路径，包含目录和文件名

    // 历史滚动
    for (uint8_t i=_backup_number-1; i>1; --i)
    {
        new_path = _log_dir + std::string("/") + _log_filename + std::string(".") + utils::CStringUtils::any2string(static_cast<int>(i));
        old_path = _log_dir + std::string("/") + _log_filename + std::string(".") + utils::CStringUtils::any2string(static_cast<int>(i-1));

        if (0 == access(old_path.c_str(), F_OK))
        {
            if (-1 == rename(old_path.c_str(), new_path.c_str()))
                fprintf(stderr, "[%d:%lu] SafeLogger rename %s to %s error: %m.\n", getpid(), pthread_self(), old_path.c_str(), new_path.c_str());
        }
        else
        {
            if (errno != ENOENT)
                fprintf(stderr, "[%d:%lu] SafeLogger access %s error: %m.\n", getpid(), pthread_self(), old_path.c_str());
        }
    }

    if (_backup_number > 0)
    {
        // 当前滚动
        new_path = _log_dir + std::string("/") + _log_filename + std::string(".1");
        if (0 == access(_log_filepath.c_str(), F_OK))
        {
            if (-1 == rename(_log_filepath.c_str(), new_path.c_str()))
                fprintf(stderr, "[%d:%lu] SafeLogger rename %s to %s error: %m\n", getpid(), pthread_self(), _log_filepath.c_str(), new_path.c_str());
        }
        else
        {
            if (errno != ENOENT)
                fprintf(stderr, "[%d:%lu] SafeLogger access %s error: %m\n", getpid(), pthread_self(), _log_filepath.c_str());
        }
    }

    // 重新创建
    printf("[%d:%lu] SafeLogger create %s\n", getpid(), pthread_self(), _log_filepath.c_str());
    int log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_EXCL, FILE_DEFAULT_PERM);
    if (-1 == log_fd)
    {
        fprintf(stderr, "[%d:%lu] SafeLogger create %s error: %m\n", getpid(), pthread_self(), _log_filepath.c_str());
    }

    LockHelper<CLock> lock_helper(_lock);
    if (-1 == close(_log_fd))
        fprintf(stderr, "[%d:%lu] SafeLogger close %s error: %m.\n", getpid(), pthread_self(), _log_filepath.c_str());
    _log_fd = log_fd;
}

SYS_NAMESPACE_END
