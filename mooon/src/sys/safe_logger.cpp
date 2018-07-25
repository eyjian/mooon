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
#include "mooon/sys/close_helper.h"
#include "mooon/sys/datetime_utils.h"
#include "mooon/sys/file_locker.h"
#include "mooon/sys/file_utils.h"
#include "mooon/utils/scoped_ptr.h"
#include "mooon/utils/string_utils.h"
#include <libgen.h>
#include <pthread.h>
#include <sstream>
#include <syslog.h>
#include <unistd.h>
SYS_NAMESPACE_BEGIN

static uint64_t get_current_thread_id()
{
    return static_cast<uint64_t>(pthread_self());
}

CSafeLogger* create_safe_logger(bool enable_program_path, uint16_t log_line_size, const std::string& suffix, bool enable_syslog) throw (CSyscallException)
{
    const std::string& log_dirpath = get_log_dirpath(enable_program_path);
    const std::string& log_filename = get_log_filename(suffix);
    CSafeLogger* logger = new CSafeLogger(log_dirpath.c_str(), log_filename.c_str(), log_line_size, enable_syslog);

    set_log_level_by_env(logger);
    enable_screen_log_by_env(logger);
    enable_trace_log_by_env(logger);
    set_log_filesize_by_env(logger);
    set_log_backup_by_env(logger);

    return logger;
}

CSafeLogger* create_safe_logger(const std::string& log_dirpath, const std::string& cpp_filename, uint16_t log_line_size, bool enable_syslog) throw (CSyscallException)
{
    const std::string& only_filename = utils::CStringUtils::extract_filename(cpp_filename);
    const std::string& log_filename = utils::CStringUtils::replace_suffix(only_filename, ".log");
    CSafeLogger* logger = new CSafeLogger(log_dirpath.c_str(), log_filename.c_str(), log_line_size, enable_syslog);

    set_log_level_by_env(logger);
    enable_screen_log_by_env(logger);
    enable_trace_log_by_env(logger);
    set_log_filesize_by_env(logger);
    set_log_backup_by_env(logger);

    return logger;
}

////////////////////////////////////////////////////////////////////////////////
CSafeLogger::CSafeLogger(const char* log_dir, const char* log_filename, uint16_t log_line_size, bool enable_syslog) throw (CSyscallException)
    :_log_fd(-1)
    ,_auto_adddot(false)
    ,_auto_newline(true)
    ,_sys_log_enabled(enable_syslog)
    ,_bin_log_enabled(false)
    ,_trace_log_enabled(false)
    ,_raw_log_enabled(false)
    ,_raw_record_time(false)
    ,_screen_enabled(false)
    ,_log_dir(log_dir)
    ,_log_filename(log_filename)
    ,_log_filepath(_log_dir + std::string("/") + _log_filename)
    ,_log_shortname(mooon::utils::CStringUtils::remove_suffix(log_filename))
{
    atomic_set(&_max_bytes, DEFAULT_LOG_FILE_SIZE);
    atomic_set(&_log_level, LOG_LEVEL_INFO);
    atomic_set(&_backup_number, DEFAULT_LOG_FILE_BACKUP_NUMBER);

    // 保证日志行最大长度不小于指定值
    _log_line_size = (log_line_size < LOG_LINE_SIZE_MIN)? LOG_LINE_SIZE_MIN: log_line_size;
    if (_log_line_size > LOG_LINE_SIZE_MAX)
    {
        _log_line_size = LOG_LINE_SIZE_MAX;
    }

    // 出错时记录系统日志
    if (_sys_log_enabled)
    {
        openlog("mooon-safe-logger", LOG_CONS|LOG_PID, 0);
    }

    _log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);
    if (-1 == _log_fd)
    {
        int errcode = errno;
        if (_sys_log_enabled)
            syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] open failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errcode));

        THROW_SYSCALL_EXCEPTION(_log_filepath, errcode, "open");
    }
}

CSafeLogger::~CSafeLogger()
{
    if (_log_fd != -1)
    {
        if (close(_log_fd) != 0)
        {
            if (_sys_log_enabled)
                syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] close failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
        }
    }

    if (_sys_log_enabled)
        closelog();
}

int CSafeLogger::get_log_level() const
{
    return atomic_read(&_log_level);
}

std::string CSafeLogger::get_log_dir() const
{
    return _log_dir;
}

std::string CSafeLogger::get_log_filename() const
{
    return _log_filename;
}

std::string CSafeLogger::get_log_filepath() const
{
    return _log_filepath;
}

std::string CSafeLogger::get_log_shortname() const
{
    return _log_shortname;
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

void CSafeLogger::enable_raw_log(bool enabled, bool record_time)
{
    _raw_log_enabled = enabled;
    _raw_record_time = record_time;
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

bool CSafeLogger::enabled_raw()
{
    return _raw_log_enabled;
}

void CSafeLogger::vlog_detail(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_detail())
        do_log(LOG_LEVEL_DETAIL, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_debug(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_detail())
        do_log(LOG_LEVEL_DEBUG, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_info(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_info())
        do_log(LOG_LEVEL_INFO, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_warn(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_warn())
        do_log(LOG_LEVEL_WARN, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_error(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_error())
        do_log(LOG_LEVEL_ERROR, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_fatal(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_fatal())
        do_log(LOG_LEVEL_FATAL, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_state(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_state())
        do_log(LOG_LEVEL_STATE, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_trace(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    if (enabled_trace())
        do_log(LOG_LEVEL_TRACE, filename, lineno, module_name, format, args);
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

void CSafeLogger::vlog_raw(const char* format, va_list& args)
{
    if (enabled_raw())
        do_log(LOG_LEVEL_RAW, NULL, -1, NULL, format, args);
}

void CSafeLogger::log_raw(const char* format, ...)
{
    if (enabled_raw())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);
        do_log(LOG_LEVEL_RAW, NULL, -1, NULL, format, args);
    }
}

void CSafeLogger::log_bin(const char* filename, int lineno, const char* module_name, const char* log, uint16_t size)
{
    if (enabled_bin())
    {
        std::string str(size*2, '\0');
        char* str_p = const_cast<char*>(str.data());
        for (uint16_t i=0; i<size; ++i)
        {
            snprintf(str_p, 3, "%02X", (int)log[i]);
            str_p += 2;
        }

        va_list args;
        do_log(LOG_LEVEL_BIN, filename, lineno, module_name, str.c_str(), args);
    }
}

bool CSafeLogger::need_rotate(int fd) const
{
    off_t file_size = CFileUtils::get_file_size(fd);
    return file_size > static_cast<off_t>(atomic_read(&_max_bytes));
}

void CSafeLogger::do_log(log_level_t log_level, const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{
    int log_real_size = 0;
    std::string log_line(_log_line_size+1, '\0');
    char* log_line_p = const_cast<char*>(log_line.data());

    if (LOG_LEVEL_RAW == log_level)
    {
        if (_raw_record_time)
        {
            char datetime[sizeof("[2012-12-12 12:12:12]")];
            CDatetimeUtils::get_current_datetime(log_line_p, sizeof(datetime), "[%04d-%02d-%02d %02d:%02d:%02d]");
            log_real_size = sizeof("[YYYY-MM-DD hh:mm:ss]") - 1;
        }

        // fix_vsnprintf()的返回值包含了结尾符在内的长度
        log_real_size += utils::CStringUtils::fix_vsnprintf(log_line_p+log_real_size, _log_line_size-log_real_size, format, args);
        --log_real_size; // 结尾符不需要写入日志文件中
    }
    else
    {
        std::stringstream log_header; // 每条日志的头
        char datetime[sizeof("2012-12-12 12:12:12/0123456789")];
        get_formatted_current_datetime(datetime, sizeof(datetime));

        // 日志头内容：[日期][线程ID/进程ID][日志级别][模块名][代码文件名][代码行号]
        log_header << "[" << datetime << "]"
                   << "[" << get_current_thread_id() << "/" << getpid() << "]"
                   << "[" << get_log_level_name(log_level) << "]";
        if (module_name != NULL)
            log_header << "[" << module_name << "]";
        if (filename != NULL)
            log_header << "[" << utils::CStringUtils::extract_filename(filename) << ":" << lineno << "]";

        int m, n;
        // 注意fix_snprintf()的返回值大小包含了结尾符
        m = utils::CStringUtils::fix_snprintf(log_line_p, _log_line_size, "%s", log_header.str().c_str());

        if (LOG_LEVEL_BIN == log_level)
            n = utils::CStringUtils::fix_snprintf(log_line_p+m-1, _log_line_size-m, "%s", format);
        else
            n = utils::CStringUtils::fix_vsnprintf(log_line_p+m-1, _log_line_size-m, format, args);
        log_real_size = m + n - 2; // 减去2个结尾符
    }

    // 是否自动添加结尾用的点号
    if (_auto_adddot)
    {
        // 如果已有结尾的点，则不再添加，以免重复
        if (log_line_p[log_real_size-1] != '.')
        {
            log_line_p[log_real_size] = '.';
            ++log_real_size;
        }
    }
    if (_auto_newline) // 是否自动换行
    {
        // 如果已有一个换行符，则不再添加
        if (log_line_p[log_real_size-1] != '\n')
        {
            log_line_p[log_real_size] = '\n';
            ++log_real_size;
        }
    }
    if (_screen_enabled) // 允许打屏
    {
        (void)write(STDOUT_FILENO, log_line_p, log_real_size);
    }

    if (false)
    {
        // 异步写入日志文件
        //log_line.release();
    }
    else
    {
        // 同步写入日志文件
        write_log(log_line_p, log_real_size);
    }
}

void CSafeLogger::rotate_log()
{
    std::string new_path;  // 滚动后的文件路径，包含目录和文件名
    std::string old_path;  // 滚动前的文件路径，包含目录和文件名

    // 历史滚动
    int backup_number = atomic_read(&_backup_number);
    for (int i=backup_number-1; i>1; --i)
    {
        new_path = _log_dir + std::string("/") + _log_filename + std::string(".") + utils::CStringUtils::any2string(static_cast<int>(i));
        old_path = _log_dir + std::string("/") + _log_filename + std::string(".") + utils::CStringUtils::any2string(static_cast<int>(i-1));

        if (0 == access(old_path.c_str(), F_OK))
        {
            if (-1 == rename(old_path.c_str(), new_path.c_str()))
            {
                if (_sys_log_enabled)
                    syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] rename to %s failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), old_path.c_str(), new_path.c_str(), strerror(errno));
            }
        }
        else
        {
            if (errno != ENOENT)
            {
                if (_sys_log_enabled)
                    syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] access failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), old_path.c_str(), strerror(errno));
            }
        }
    }

    if (backup_number > 0)
    {
        // 当前滚动
        new_path = _log_dir + std::string("/") + _log_filename + std::string(".1");
        if (0 == access(_log_filepath.c_str(), F_OK))
        {
            if (-1 == rename(_log_filepath.c_str(), new_path.c_str()))
            {
                if (_sys_log_enabled)
                    syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] rename to %s failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), new_path.c_str(), strerror(errno));
            }
        }
        else
        {
            if (errno != ENOENT)
            {
                if (_sys_log_enabled)
                    syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] access failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
            }
        }
    }
}

void CSafeLogger::write_log(const char* log_line, int log_line_size)
{
    CloseHelper<int> log_fd(prepare_log_fd());
    if (-1 == log_fd.get())
    {
        return; // 没法继续
    }

    int bytes = write(log_fd.get(), log_line, log_line_size);
    if (-1 == bytes)
    {
        if (_sys_log_enabled)
            syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] write failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
    }
    else if (0 == bytes)
    {
        if (_sys_log_enabled)
            syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] write failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
    }
    else if (bytes > 0)
    {
        try
        {
            // 判断是否需要滚动
            if (need_rotate(log_fd.get()))
            {
                std::string lock_path = _log_dir + std::string("/.") + _log_filename + std::string(".lock");
                FileLocker file_locker(lock_path.c_str(), true); // 确保这里一定加锁

                // _fd可能已被其它进程或线程滚动了，所以这里需要重新open一下
                int new_log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);
                if (-1 == new_log_fd)
                {
                    if (_sys_log_enabled)
                        syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] open failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
                }
                else
                {
                    try
                    {
                        if (need_rotate(new_log_fd))
                            rotate_log();

                        // 不管谁滚动的，都需要重设_log_fd，
                        // 原因是如果是由其它进程滚动的，则当前进程的_log_fd是不会变化的
                        WriteLockHelper rlh(_read_write_lock);
                        if (0 == close(_log_fd))
                            _log_fd = new_log_fd;
                        else
                            close(new_log_fd);
                    }
                    catch (CSyscallException& syscall_ex)
                    {
                        if (_sys_log_enabled)
                            syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
                    }
                }
            }
        }
        catch (CSyscallException& syscall_ex)
        {
            if (_sys_log_enabled)
                syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
        }
    }
}

int CSafeLogger::prepare_log_fd()
{
    int log_fd = -1;
    {
        ReadLockHelper rlh(_read_write_lock);
        if (_log_fd != -1)
            log_fd = dup(_log_fd);
    }

    if (-1 == log_fd)
    {
        // 可能是因为文件还未打开过
        WriteLockHelper rlh(_read_write_lock);
        if (-1 == _log_fd)
        {
            _log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);
            if (-1 == _log_fd)
            {
                if (_sys_log_enabled)
                    syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] open failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), strerror(errno));
                return -1; // 没法继续玩
            }
        }

        log_fd = dup(_log_fd);
        if (-1 == log_fd)
        {
            if (_sys_log_enabled)
                syslog(LOG_ERR, "[%s:%d][%u][%" PRIu64"][%s] dup (%d) failed: %s\n", __FILE__, __LINE__, getpid(), get_current_thread_id(), _log_filepath.c_str(), _log_fd, strerror(errno));
            return -1; // 没法继续玩
        }
    }

    return log_fd;
}

SYS_NAMESPACE_END
