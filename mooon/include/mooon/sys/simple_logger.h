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
 * 独立的单个头文件，可即时独立使用，只要定义了宏NOT_WITH_MOOON，即不依赖于mooon
 * 简单的写日志类，多进程安全，但非多线程安全，只提供按大小滚动功能
 * 不追求功能，也不追求性能，只求简单，若要功能强、性能高，可以使用CLogger
 *
 * 使用方法：
 * 1) 构造一个CSimpleLogger对象
 *    CSimpleLogger logger(".", "test.log", 1024*1024, 10);
 * 2) 调用print方法写日志
 *    logger.info(__FILE__, __LINE__, "%s\n", "test");
 */
#ifndef MOOON_SYS_SIMPLE_LOGGER_H
#define MOOON_SYS_SIMPLE_LOGGER_H
#include "mooon/sys/file_locker.h"
#include <fcntl.h>
#include <sstream>
#include <stdarg.h>
#include <string>
#include <syslog.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
SYS_NAMESPACE_BEGIN

/**
 * 便于使用的日志宏
 */

// Raw
#define __SIMPLE_LOG_RAW(logger, format, ...) \
do { \
    if (NULL == logger) { \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } \
    else { \
        logger->raw(format, ##__VA_ARGS__); \
    } \
} while(false)

// Debug
#define __SIMPLE_LOG_DEBUG(logger, format, ...) \
do { \
    if (NULL == logger) { \
        printf("[DEBUG][%s:%d]", __FILE__, __LINE__); \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } \
    else if (logger->enabled_debug()) { \
        logger->debug(__FILE__, __LINE__, format, ##__VA_ARGS__); \
    } \
} while(false)

// Info
#define __SIMPLE_LOG_INFO(logger, format, ...) \
do { \
    if (NULL == logger) { \
        printf("[INFO][%s:%d]", __FILE__, __LINE__); \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } \
    else if (logger->enabled_info()) { \
        logger->info(__FILE__, __LINE__, format, ##__VA_ARGS__); \
    } \
} while(false)

// Error
#define __SIMPLE_LOG_ERROR(logger, format, ...) \
do { \
    if (NULL == logger) { \
        printf("[ERROR][%s:%d]", __FILE__, __LINE__); \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } \
    else if (logger->enabled_error()) { \
        logger->error(__FILE__, __LINE__, format, ##__VA_ARGS__); \
    } \
} while(false)

// Warning
#define __SIMPLE_LOG_WARNING(logger, format, ...) \
do { \
    if (NULL == logger) { \
        printf("[WARNING][%s:%d]", __FILE__, __LINE__); \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } \
    else if (logger->enabled_warning()) { \
        logger->warning(__FILE__, __LINE__, format, ##__VA_ARGS__); \
    } \
} while(false)

// Fetal
#define __SIMPLE_LOG_FETAL(logger, format, ...) \
do { \
    if (NULL == logger) { \
        printf("[FETAL][%s:%d]", __FILE__, __LINE__); \
        printf(format, ##__VA_ARGS__); \
        printf("\n"); \
    } \
    else if (logger->enabled_fetal()) { \
        logger->fetal(__FILE__, __LINE__, format, ##__VA_ARGS__); \
    } \
} while(false)
    
// 默认的文件模式
#ifndef FILE_DEFAULT_PERM
#define FILE_DEFAULT_PERM (S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)
#endif // FILE_DEFAULT_PERM

/***
 * 支持按文件大小滚动日志记录器，支持多进程写同一个日志文件
 */
class CSimpleLogger
{
public:
    /***
      * 构造一个CSimpleLogger，并创建或打开日志文件
      * @log_dir 日志存放的目录，不需要以斜杠结尾，目录必须已经存在
      * @filename 日志的文件名，不包含目录部分，
      *           由log_dir和filename共同组成日志文件路径
      * @log_size 每个日志文件的大小，单位为字节数，如果小于1024，则会被强制为1024
      * @log_numer 日志滚动的个数
      * @record_size 单条日志的大小，超过会被截断，单位为字节数，如果小于1024，则会被强制为1024
      */
    explicit CSimpleLogger(const std::string& log_dir,
                           const std::string& filename,
                           uint32_t log_size = 1024*1024*100,
                           uint8_t log_numer = 10,
                           uint16_t record_size = 8192);
    ~CSimpleLogger();

    /** 日志文件是否创建或打开成功 */
    bool is_ok() const;

    /** 输出日志，象printf一样使用，不自动加换行符 */
    void raw(const char* format, ...) __attribute__((format(printf, 2, 3)));
    void debug(const char* file, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
    void info(const char* file, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
    void error(const char* file, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
    void warning(const char* file, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
    void fetal(const char* file, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));

    /** 得到含文件名的日志文件路径*/
    const std::string& get_log_filepath() const;

    /** 设置tag */
    template <typename IntType>
    void set_tag1(IntType tag);
    void set_tag1(const std::string& tag);
    template <typename IntType>
    void set_tag2(IntType tag);
    void set_tag2(const std::string& tag);

    /** 判断某种级别的日志是否可输出 */
    bool enabled_debug() const;
    bool enabled_info() const;
    bool enabled_error() const;
    bool enabled_warning() const;
    bool enabled_fetal() const;

private:
    void print(const char* file, int line, const char* level, const char* format, va_list& ap);
    bool need_rotate() const;        /** 是否需要滚动了 */
    bool need_rotate(int fd) const;  /** 是否需要滚动了 */
    void reset();                    /** 复位状态值 */
    void rotate_log();               /** 滚动日志 */

private:    
    int _fd;                   /** 当前正在写的日志文件描述符 */
    std::string _log_dir;      /** 日志存放目录 */
    std::string _filename;     /** 日志文件名，不包含目录部分 */
    uint32_t _log_size;        /** 单个日志文件的大小 */
    uint8_t _log_numer;        /** 日志滚动的个数 */
    uint16_t _record_size;     /** 单条日志的大小，单位为字节数 */
    std::string _log_filepath; /** 含文件名的日志文件路径 */
    std::string _tag1;         /** 自定义的标记1 */
    std::string _tag2;         /** 自定义的标记2 */
};

/** 万用类型转换函数 */
template <typename Any>
inline std::string any2string(Any any)
{
    std::stringstream s;
    s << any;
    return s.str();
}

inline int fix_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int expected = vsnprintf(str, size, format, ap);

    if (expected < static_cast<int>(size))
        return expected + 1;

    return static_cast<int>(size);
}

inline void get_current_datetime(char* datetime_buffer, size_t datetime_buffer_size)
{
    struct tm result;
    time_t now = time(NULL);

    localtime_r(&now, &result);
    snprintf(datetime_buffer, datetime_buffer_size
        ,"%04d-%02d-%02d %02d:%02d:%02d"
        ,result.tm_year+1900, result.tm_mon+1, result.tm_mday
        ,result.tm_hour, result.tm_min, result.tm_sec);
}

inline std::string get_current_datetime()
{
    char datetime_buffer[sizeof("YYYY-MM-DD HH:SS:MM")];
    get_current_datetime(datetime_buffer, sizeof(datetime_buffer));
    return datetime_buffer;
}

inline off_t get_file_size(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
    {
        return -1;
    }
    
    // 判断是否为文件
    if (!S_ISREG(buf.st_mode))
    {
        return -1;
    }

    return buf.st_size;
}

inline CSimpleLogger::CSimpleLogger(
                     const std::string& log_dir,
                     const std::string& filename,
                     uint32_t log_size,
                     uint8_t log_numer,
                     uint16_t record_size)
     : _fd(-1),
       _log_dir(log_dir),
       _filename(filename),
       _log_size(log_size),
       _log_numer(log_numer),
       _record_size(record_size)
{
    _log_filepath = _log_dir + std::string("/") + _filename;
    _fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);

    if (_fd != -1)
    {
        // 不能太小气了
        if (_log_size < 1024)
        {
            _log_size = 1024;
        }

        // 同样不能太小气
        if (_record_size < 1024)
        {
            _record_size = 1024;
        }
    }
}

inline CSimpleLogger::~CSimpleLogger()
{
    if (_fd != -1)
        close(_fd);
}

inline bool CSimpleLogger::is_ok() const
{
    return _fd != -1;
}

inline void CSimpleLogger::raw(const char* format, ...)
{
    if (_fd != -1)
    {
        va_list ap;
        va_start(ap, format);

        print(NULL, 0, NULL, format, ap);
        va_end(ap);
    }
}

inline void CSimpleLogger::debug(const char* file, int line, const char* format, ...)
{
    if (_fd != -1)
    {
        va_list ap;
        va_start(ap, format);

        print(file, line, "DEBUG", format, ap);
        va_end(ap);
    }
}

inline void CSimpleLogger::info(const char* file, int line, const char* format, ...)
{
    if (_fd != -1)
    {
        va_list ap;
        va_start(ap, format);

        print(file, line, "INFO", format, ap);
        va_end(ap);
    }
}

inline void CSimpleLogger::error(const char* file, int line, const char* format, ...)
{
    if (_fd != -1)
    {
        va_list ap;
        va_start(ap, format);

        print(file, line, "ERROR", format, ap);
        va_end(ap);
    }
}

inline void CSimpleLogger::warning(const char* file, int line, const char* format, ...)
{
    if (_fd != -1)
    {
        va_list ap;
        va_start(ap, format);

        print(file, line, "WARNING", format, ap);
        va_end(ap);
    }
}

inline void CSimpleLogger::fetal(const char* file, int line, const char* format, ...)
{
    if (_fd != -1)
    {
        va_list ap;
        va_start(ap, format);

        print(file, line, "FETAL", format, ap);
        va_end(ap);
    }
}

inline const std::string& CSimpleLogger::get_log_filepath() const
{
    return _log_filepath;
}

inline bool CSimpleLogger::enabled_debug() const
{
    return true;
}

inline bool CSimpleLogger::enabled_info() const
{
    return true;
}

inline bool CSimpleLogger::enabled_error() const
{
    return true;
}

inline bool CSimpleLogger::enabled_warning() const
{
    return true;
}

inline bool CSimpleLogger::enabled_fetal() const
{
    return true;
}

// 格式：[YYYY-MM-DD hh:mm:ss][pid][Level][tag][file:line]log，其中tag是可选的
inline void CSimpleLogger::print(const char* file, int line, const char* level, const char* format, va_list& ap)
{
    if (_fd != -1)
    {
        std::string log_header; // 存储每条日志头

        // raw日志，不加头
        if (level != NULL)
        {
            // date&time
            log_header = std::string("[") + get_current_datetime() + std::string("]");
            // level
            log_header += std::string("[") + std::string(level) + std::string("]");
            // pid
            log_header += std::string("[") + any2string(getpid()) + std::string("]");

            if (!_tag1.empty()) // tag1
            {
                log_header += std::string("[") + _tag1 + std::string("]");
            }
            if (!_tag2.empty()) // tag2
            {
                log_header += std::string("[") + _tag2 + std::string("]");
            }
            if (file != NULL) // file&line
            {
                log_header += std::string("[") +
                              std::string(file) + std::string(":") + any2string(line) +
                              std::string("]");
            }
        }

        // 自动添加换行符，所以加1
        size_t content_size = _record_size - log_header.size() + 1;        
        char* content_buffer = new char[content_size];

        // writev参数
        struct iovec iov[2];

        // log header
        iov[0].iov_base = const_cast<char*>(log_header.data());
        iov[0].iov_len = log_header.size(); // 不含结尾符

        // log content
        iov[1].iov_base = content_buffer;
        iov[1].iov_len = fix_vsnprintf(content_buffer, content_size, format, ap);
        content_buffer[iov[1].iov_len - 1] = '\n'; // 添加换行符

        // 写入文件
        ssize_t bytes_writed = writev(_fd, iov, sizeof(iov)/sizeof(iov[0]));
        delete []content_buffer;

        if (bytes_writed > 0)
        {
            // 滚动处理
            if (need_rotate())
            {    
                // 加文件锁
                std::string lock_path = _log_dir + std::string("/.") + _filename + std::string(".lock");
                FileLocker file_locker(lock_path.c_str(), true); // 确保这里一定加锁
                
                // _fd可能已被其它进程滚动了，所以这里需要重新open一下
                std::string log_filepath = _log_dir + std::string("/") + _filename;
                int fd = open(log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);

                // 需要再次判断，原因是可能其它进程已处理过了
                if (need_rotate(fd))
                {
                    close(fd);
                    rotate_log();
                }
                else // 其它进程完成了滚动
                {
                    close(_fd);
                    _fd = fd;
                }
            }
        }
    }
}

inline bool CSimpleLogger::need_rotate() const
{
    return get_file_size(_fd) > static_cast<off_t>(_log_size);
}

inline bool CSimpleLogger::need_rotate(int fd) const
{
    return get_file_size(fd) > static_cast<off_t>(_log_size);
}

inline void CSimpleLogger::rotate_log()
{
    std::string new_path;  // 滚动后的文件路径，包含目录和文件名
    std::string old_path;  // 滚动前的文件路径，包含目录和文件名

    // 轮回，一切重新开始
    reset();

    // 历史滚动
    for (uint8_t i=_log_numer-1; i>0; --i)
    {
        new_path = _log_dir + std::string("/") + _filename + std::string(".") + any2string(static_cast<int>(i));
        old_path = _log_dir + std::string("/") + _filename + std::string(".") + any2string(static_cast<int>(i-1));

        if (0 == access(old_path.c_str(), F_OK))
        {
            rename(old_path.c_str(), new_path.c_str());
        }
    }

    if (_log_numer > 0)
    {
        // 当前滚动
        new_path = _log_dir + std::string("/") + _filename + std::string(".1");
        old_path = _log_dir + std::string("/") + _filename;
        if (0 == access(old_path.c_str(), F_OK))
        {
            rename(old_path.c_str(), new_path.c_str());
        }
    }

    // 重新创建
    printf("create %s\n", old_path.c_str());
    _fd = open(old_path.c_str(), O_WRONLY|O_CREAT|O_EXCL, FILE_DEFAULT_PERM);
}

inline void CSimpleLogger::reset()
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

template <typename IntType>
inline void CSimpleLogger::set_tag1(IntType tag)
{
    _tag1 = any2string(tag);
}

inline void CSimpleLogger::set_tag1(const std::string& tag)
{
    _tag1 = tag;
}

template <typename IntType>
inline void CSimpleLogger::set_tag2(IntType tag)
{
    _tag2 = any2string(tag);
}

inline void CSimpleLogger::set_tag2(const std::string& tag)
{
    _tag2 = tag;
}

/***
  * 测试代码
#include "simple_logger.h"
int main()
{
    int milliseconds = 10;
    CSimpleLogger logger(".", "test.log", 10240);
    
    for (int i=0; i<100000; ++i)
    {
        logger.debug(__FILE__, __LINE__, "%d ==> abcdefghijklmnopqrestuvwxyz.\n", i);
        
        struct timespec ts = { milliseconds / 1000, (milliseconds % 1000) * 1000000 };
        while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
    }
        
    return 0;
}
*/

SYS_NAMESPACE_END
#endif // MOOON_SYS_SIMPLE_LOGGER_H
