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
#ifndef MOOON_SYS_LOG_H
#define MOOON_SYS_LOG_H
#include <mooon/sys/config.h>
#include <mooon/utils/print_color.h>
#include <stdio.h>
SYS_NAMESPACE_BEGIN

class ILogger;

/** 不要修改下面的常量值，而应当通过对应的方法去修改
  * 这些常量值主要是方便多模块共享，故放在这个公有头文件当中
  */
enum
{
    LOG_LINE_SIZE_MIN              = 256,        /** 日志行最小长度 */
    LOG_LINE_SIZE_MAX              = 32768,      /** 日志行最大长度(32K) ，最大不能超过64K，因为使用2字节无符号整数存储的 */
    DEFAULT_LOG_FILE_SIZE          = 524288000,  /** 默认的单个日志文件大小(500MB) */
    DEFAULT_LOG_FILE_BACKUP_NUMBER = 10          /** 默认的日志文件备份个数 */
};

/** 定义日志级别 */
typedef enum
{
    LOG_LEVEL_DETAIL = 0,
    LOG_LEVEL_DEBUG  = 1,
    LOG_LEVEL_INFO   = 2,
    LOG_LEVEL_WARN   = 3,
    LOG_LEVEL_ERROR  = 4,
    LOG_LEVEL_FATAL  = 5,    
    LOG_LEVEL_STATE  = 6,  /** 仅输出状态数据 */
    LOG_LEVEL_TRACE  = 7,
    LOG_LEVEL_RAW    = 8,  /** 裸日志，传入什么输出什么 */
    LOG_LEVEL_BIN    = 9
}log_level_t;

/** 通过日志级别名得到日志级别 */
extern log_level_t get_log_level(const char* level_name);

// 通过日志级别得到日志级别名，如果传入错误的日志级别，则返回NULL
// 有效的日志名为（全大写）：
// DETAIL, DEBUG, INFO, WARN, ERROR, FATAL, STATE, TRACE
extern const char* get_log_level_name(log_level_t log_level);

// 根据程序文件得到日志文件名，结果不包含目录
// 如果suffix为空：
// 1) 假设程序文件名为mooon，则返回结果为mooon.log
// 2) 假设程序文件名为mooon.exe，则返回结果为mooon.log
//
// 如果suffix不为空，假设为6789：
// 1) 假设程序文件名为mooon，则返回结果为mooon_6789.log
// 2) 假设程序文件名为mooon.exe，则返回结果为mooon_6789.log
extern std::string get_log_filename(const std::string& suffix=std::string(""));

// 根据程序文件得到日志文件的目录路径，不包含日志文件名
extern std::string get_log_dirpath(bool enable_program_path=true);

// 根据程序文件得到日志文件路径，返回结果包含目录和文件名
// 假设程序文件所在路径为：/data/mooon/bin/test，
// 同时存在目录/data/mooon/log，则日志自动放在该目录下，
// 否则当enable_program_path为true时，日志放在/data/mooon/bin目录下。
// 如果不存在目录/data/mooon/log，且enable_program_path为false，则函数返回空字符串
extern std::string get_log_filepath(bool enable_program_path=true, const std::string& suffix=std::string(""));

// 根据环境变量名MOOON_LOG_LEVEL来设置日志级别
// 如果没有设置环境变量MOOON_LOG_LEVEL，则set_log_level_by_env()什么也不做
// 日志文件名只能为下列值：
// DETAIL, DEBUG, INFO, WARN, ERROR, FATAL, STATE, TRACE
// 如果非这些值，则默认为INFO
extern void set_log_level_by_env(ILogger* logger);

// 根据环境变量名MOOON_LOG_SCREEN控制是否在屏幕上打印日志，只会值为1时才会屏幕上打印日志
extern void enable_screen_log_by_env(ILogger* logger);

// 根据环境变量名MOOON_LOG_TRACE控制是否记录trace日志
extern void enable_trace_log_by_env(ILogger* logger);

// 根据环境变量名MOOON_LOG_FILESIZE控制单个日志文件大小
extern void set_log_filesize_by_env(ILogger* logger);

// 根据环境变量名MOOON_LOG_BACKUP控制日志文件备份个数
extern void set_log_backup_by_env(ILogger* logger);

/**
  * 日志器接口，提供常见的写日志功能
  */
class ILogger
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~ILogger() {}
    virtual int get_log_level() const { return -1; }
    virtual std::string get_log_dir() const { return std::string(""); }
    virtual std::string get_log_filename() const { return std::string(""); }
    virtual std::string get_log_filepath() const { return std::string(""); }
    virtual std::string get_log_shortname() const { return std::string(""); } /* 不包含后缀的文件名 */

    /** 是否允许同时在标准输出上打印日志 */
    virtual void enable_screen(bool enabled) {}
    /** 是否允许二进制日志，二进制日志必须通过它来打开 */
    virtual void enable_bin_log(bool enabled) {}
    /** 是否允许跟踪日志，跟踪日志必须通过它来打开 */
    virtual void enable_trace_log(bool enabled) {}
    /** 是否允许祼日志，祼日志必须通过它来打开 */
    virtual void enable_raw_log(bool enabled, bool record_time=false) {}
    /** 是否自动在一行后添加结尾的点号，如果最后已经有点号或换符符，则不会再添加 */
    virtual void enable_auto_adddot(bool enabled) {}
    /** 是否自动添加换行符，如果已经有换行符，则不会再自动添加换行符 */
    virtual void enable_auto_newline(bool enabled) {}   
    /** 设置日志级别，跟踪日志级别不能通过它来设置 */
    virtual void set_log_level(log_level_t log_level) {}
    /** 设置单个文件的最大建议大小 */
    virtual void set_single_filesize(uint32_t filesize) {}
    /** 设置日志文件备份个数，不包正在写的日志文件 */
    virtual void set_backup_number(uint16_t backup_number) {}

    /** 是否允许二进制日志 */
    virtual bool enabled_bin() { return false; }
    /** 是否允许Detail级别日志 */
    virtual bool enabled_detail() { return false; }
    /** 是否允许Debug级别日志 */
    virtual bool enabled_debug() { return false; }
    /** 是否允许Info级别日志 */
    virtual bool enabled_info() { return false; }
    /** 是否允许Warn级别日志 */
    virtual bool enabled_warn() { return false; }
    /** 是否允许Error级别日志 */
    virtual bool enabled_error() { return false; }
    /** 是否允许Fatal级别日志 */
    virtual bool enabled_fatal() { return false; }
    /** 是否允许输出状态日志 */
    virtual bool enabled_state() { return false; }
    /** 是否允许Trace级别日志 */
    virtual bool enabled_trace() { return false; }
    /** 是否允许Raw级别日志 */
    virtual bool enabled_raw() { return false; }

    virtual void vlog_detail(const char* filename, int lineno, const char* module_name, const char* format, va_list& args) {}
    // 由于隐含了this参数（总是1个参数），所以format为第5个参数，...从第6个参数开始
    virtual void log_detail(const char* filename, int lineno, const char* module_name, const char* format, ...)  __attribute__((format(printf, 5, 6))) {}

    virtual void vlog_debug(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)  {}
    virtual void log_debug(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))   {}

    virtual void vlog_info(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)   {}
    virtual void log_info(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))    {}

    virtual void vlog_warn(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)   {}
    virtual void log_warn(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))    {}

    virtual void vlog_error(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)  {}
    virtual void log_error(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))   {}

    virtual void vlog_fatal(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)  {}
    virtual void log_fatal(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))   {}

    virtual void vlog_state(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)  {}
    virtual void log_state(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))   {}

    virtual void vlog_trace(const char* filename, int lineno, const char* module_name, const char* format, va_list& args)  {}
    virtual void log_trace(const char* filename, int lineno, const char* module_name, const char* format, ...) __attribute__((format(printf, 5, 6)))   {}

    /** 写裸日志 */
    virtual void vlog_raw(const char* format, va_list& ap) {}
    virtual void log_raw(const char* format, ...) __attribute__((format(printf, 2, 3))) {}

    /** 写二进制日志 */
    virtual void log_bin(const char* filename, int lineno, const char* module_name, const char* log, uint16_t size) {}
};

//////////////////////////////////////////////////////////////////////////
// 日志宏，方便记录日志
extern ILogger* g_logger; // 只是声明，不是定义，不能赋值哦！
extern bool g_null_print_screen; // 当g_logger为空时是否打屏，默认为true

#define __MYLOG_DETAIL(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
	        fprintf(stderr, "[DETAIL][%s:%d]", __FILE__, __LINE__); \
	        fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_detail()) { \
		logger->log_detail(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

// C++11要求PRINT_COLOR_NONE前和PRINT_COLOR_DARY_GRAY后保留一个空格，否则编译报警：
// invalid suffix on literal; C++11 requires a space between literal and identifier [-Wliteral-suffix]
#define __MYLOG_DEBUG(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, PRINT_COLOR_DARY_GRAY "[DEBUG][%s:%d]" PRINT_COLOR_NONE, __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_debug()) { \
		logger->log_debug(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_INFO(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, "[INFO][%s:%d]", __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_info()) { \
		logger->log_info(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_WARN(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, PRINT_COLOR_YELLOW "[WARN][%s:%d]" PRINT_COLOR_NONE, __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_warn()) { \
		logger->log_warn(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_ERROR(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, PRINT_COLOR_RED "[ERROR][%s:%d]" PRINT_COLOR_NONE, __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_error()) { \
		logger->log_error(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_FATAL(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, PRINT_COLOR_BROWN "[FATAL][%s:%d]" PRINT_COLOR_NONE, __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_fatal()) { \
		logger->log_fatal(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_STATE(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, "[STATE][%s:%d]", __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_state()) { \
		logger->log_state(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_TRACE(logger, module_name, format, ...) \
do { \
	if (NULL == logger) { \
	    if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, "[TRACE][%s:%d]", __FILE__, __LINE__); \
            fprintf(stderr, format, ##__VA_ARGS__); \
	    } \
	} \
	else if (logger->enabled_trace()) { \
		logger->log_trace(__FILE__, __LINE__, module_name, format, ##__VA_ARGS__); \
	} \
} while(false)

#define __MYLOG_RAW(logger, format, ...) \
do { \
    if (NULL == logger) { \
        if (::mooon::sys::g_null_print_screen) { \
            fprintf(stderr, format, ##__VA_ARGS__); \
        } \
    } \
    else if (logger->enabled_raw()) { \
        logger->log_raw(format, ##__VA_ARGS__); \
    } \
} while(false)

#define __MYLOG_BIN(logger, module_name, log, size) \
do { \
    if ((logger != NULL) && logger->enabled_bin()) \
        logger->log_bin(__FILE__, __LINE__, module_name, log, size); \
} while(false)

#define __MYLOG_DETAIL_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_detail())))
#define __MYLOG_DEBUG_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_debug())))
#define __MYLOG_INFO_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_info())))
#define __MYLOG_ERROR_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_error())))
#define __MYLOG_WARN_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_warn())))
#define __MYLOG_FATAL_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_fatal())))
#define __MYLOG_STATE_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_state())))
#define __MYLOG_TRACE_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_trace())))
#define __MYLOG_RAW_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_raw())))
#define __MYLOG_BIN_ENABLE(logger) (((NULL == logger) && ::mooon::sys::g_null_print_screen) || ((logger != NULL) && (logger->enabled_bin())))

#define MYLOG_ENABLE()               (::mooon::sys::g_logger != NULL)
#define MYLOG_BIN(log, size)         __MYLOG_BIN(::mooon::sys::g_logger, NULL, log, size)
#define MYLOG_RAW(format, ...)       __MYLOG_RAW(::mooon::sys::g_logger, format, ##__VA_ARGS__)
#define MYLOG_TRACE(format, ...)     __MYLOG_TRACE(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_STATE(format, ...)     __MYLOG_STATE(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_FATAL(format, ...)     __MYLOG_FATAL(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_ERROR(format, ...)     __MYLOG_ERROR(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_WARN(format, ...)      __MYLOG_WARN(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_INFO(format, ...)      __MYLOG_INFO(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_DEBUG(format, ...)     __MYLOG_DEBUG(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)
#define MYLOG_DETAIL(format, ...)    __MYLOG_DETAIL(::mooon::sys::g_logger, NULL, format, ##__VA_ARGS__)

#define MYLOG_DETAIL_ENABLE() __MYLOG_DETAIL_ENABLE(::mooon::sys::g_logger)
#define MYLOG_DEBUG_ENABLE() __MYLOG_DEBUG_ENABLE(::mooon::sys::g_logger)
#define MYLOG_INFO_ENABLE() __MYLOG_INFO_ENABLE(::mooon::sys::g_logger)
#define MYLOG_ERROR_ENABLE() __MYLOG_ERROR_ENABLE(::mooon::sys::g_logger)
#define MYLOG_WARN_ENABLE() __MYLOG_WARN_ENABLE(::mooon::sys::g_logger)
#define MYLOG_FATAL_ENABLE() __MYLOG_FATAL_ENABLE(::mooon::sys::g_logger)
#define MYLOG_STATE_ENABLE() __MYLOG_STATE_ENABLE(::mooon::sys::g_logger)
#define MYLOG_TRACE_ENABLE() __MYLOG_TRACE_ENABLE(::mooon::sys::g_logger)
#define MYLOG_RAW_ENABLE() __MYLOG_RAW_ENABLE(::mooon::sys::g_logger)
#define MYLOG_BIN_ENABLE() __MYLOG_BIN_ENABLE(::mooon::sys::g_logger)

SYS_NAMESPACE_END
#endif // MOOON_SYS_LOG_H
