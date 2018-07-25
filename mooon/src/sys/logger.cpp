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
 */
#include "sys/logger.h"
#include "sys/datetime_utils.h"
#include "sys/dir_utils.h"
#include "sys/utils.h"
#include "utils/string_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#if HAVE_UIO_H==1 // 需要使用sys_config.h中定义的HAVE_UIO_H宏
#include <sys/uio.h>
#endif // HAVE_UIO_H

#define LOG_FLAG_BIN  0x01
#define LOG_FLAG_TEXT 0x02
SYS_NAMESPACE_BEGIN

// 在sys/log.h中声明
ILogger* g_logger = NULL;
bool g_null_print_screen = false; // 当g_logger为空时是否打屏

/** 日志级别名称数组，最大名称长度为8个字符，如果长度不够，编译器会报错 */
static char log_level_name_array[][8] = { "DETAIL", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "STATE", "TRACE", "RAW", "BIN" };

log_level_t get_log_level(const char* level_name)
{
    if (NULL == level_name) return LOG_LEVEL_DEBUG;
    if (0 == strcasecmp(level_name, "DETAIL")) return LOG_LEVEL_DETAIL;
    if (0 == strcasecmp(level_name, "DEBUG"))  return LOG_LEVEL_DEBUG;
    if (0 == strcasecmp(level_name, "TRACE"))  return LOG_LEVEL_TRACE;
    if (0 == strcasecmp(level_name, "INFO"))   return LOG_LEVEL_INFO;
    if (0 == strcasecmp(level_name, "WARN"))   return LOG_LEVEL_WARN;
    if (0 == strcasecmp(level_name, "ERROR"))  return LOG_LEVEL_ERROR;
    if (0 == strcasecmp(level_name, "FATAL"))  return LOG_LEVEL_FATAL;
    if (0 == strcasecmp(level_name, "STATE"))  return LOG_LEVEL_STATE;

    return LOG_LEVEL_INFO;
}

const char* get_log_level_name(log_level_t log_level)
{
    if ((log_level < LOG_LEVEL_DETAIL) || (log_level > LOG_LEVEL_BIN)) return NULL;
    return log_level_name_array[log_level];
}

std::string get_log_filename(const std::string& suffix)
{
    const std::string program_short_name = CUtils::get_program_short_name();
    std::string log_filename = utils::CStringUtils::remove_suffix(program_short_name);
    if (suffix.empty())
        return log_filename + std::string(".log");
    else
        return log_filename + std::string("_") + suffix + std::string(".log");
}

std::string get_log_dirpath(bool enable_program_path)
{
    std::string log_dirpath;
    std::string program_path = CUtils::get_program_path();

    try
    {
        log_dirpath = program_path + std::string("/../log");
        if (!CDirUtils::exist(log_dirpath))
        {
            if (enable_program_path)
                log_dirpath = program_path;
        }
    }
    catch (CSyscallException& syscall_ex)
    {
        if (enable_program_path)
            log_dirpath = program_path;

        //fprintf(stderr, "get_log_filepath failed: %s\n", syscall_ex.str().c_str());
    }

    return log_dirpath;
}

std::string get_log_filepath(bool enable_program_path, const std::string& suffix)
{
    std::string log_dirpath = get_log_dirpath(enable_program_path);
    return log_dirpath + std::string("/") + get_log_filename(suffix);
}

void set_log_level_by_env(ILogger* logger)
{
    const char* c_log_level = getenv("MOOON_LOG_LEVEL");

    if (c_log_level != NULL)
    {
        log_level_t log_level = get_log_level(c_log_level);
        logger->set_log_level(log_level);
    }
}

void enable_screen_log_by_env(ILogger* logger)
{
    const char* c_log_screen = getenv("MOOON_LOG_SCREEN");

    if ((c_log_screen != NULL) && (0 == strcmp(c_log_screen, "1")))
        logger->enable_screen(true);
}

void enable_trace_log_by_env(ILogger* logger)
{
    const char* c_log_trace = getenv("MOOON_LOG_TRACE");

    if (c_log_trace != NULL)
    {
        if ((c_log_trace != NULL) && (0 == strcmp(c_log_trace, "1")))
            logger->enable_trace_log(true);
    }
}

void set_log_filesize_by_env(ILogger* logger)
{
    const char* c_log_filesize = getenv("MOOON_LOG_FILESIZE");

    if (c_log_filesize != NULL)
    {
        uint32_t log_filesize = utils::CStringUtils::string2int<uint32_t>(c_log_filesize);
        if (log_filesize >= 1024)
            logger->set_single_filesize(log_filesize);
    }
}

void set_log_backup_by_env(ILogger* logger)
{
    const char* c_log_backup = getenv("MOOON_LOG_BACKUP");
    if (c_log_backup != NULL)
    {
        uint16_t log_backup = utils::CStringUtils::string2int<uint16_t>(c_log_backup);
        if (log_backup > 0)
            logger->set_backup_number(log_backup);
    }
}

//////////////////////////////////////////////////////////////////////////
// CLogProber
CLogProber::CLogProber()
{
    if (-1 == pipe(_pipe_fd))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "pipe");
}

CLogProber::~CLogProber()
{
    if (_pipe_fd[0] != -1)
    {
        close(_pipe_fd[0]);
        close(_pipe_fd[1]);
    }
}

void CLogProber::send_signal()
{
    char c = 'x';

    while (true)
    {
        if (-1 == write(_pipe_fd[1], &c, 1))
        {
            if (EINTR == Error::code()) continue;
            THROW_SYSCALL_EXCEPTION(NULL, errno, "write");
        }

        break;
    }
}

void CLogProber::read_signal(int signal_number)
{
    char signals[LOG_NUMBER_WRITED_ONCE];

    while (true)
    {
        if (-1 == read(_pipe_fd[0], reinterpret_cast<void*>(signals), static_cast<size_t>(signal_number)))
        {
            if (EINTR == Error::code()) continue;
            THROW_SYSCALL_EXCEPTION(NULL, errno, "read");
        }

        break;
    }
}

//////////////////////////////////////////////////////////////////////////
CLock CLogger::_thread_lock;
CLogThread* CLogger::_log_thread = NULL;

CLogger::CLogger(uint16_t log_line_size)
    :_log_fd(-1)
    ,_auto_adddot(false)
    ,_auto_newline(true)
    ,_bin_log_enabled(false)
    ,_trace_log_enabled(false)
    ,_registered(false)
    ,_destroying(false)
    ,_screen_enabled(false)    
    ,_current_bytes(0)
    ,_log_queue(NULL)
    ,_waiter_number(0)
{    
    atomic_set(&_max_bytes, DEFAULT_LOG_FILE_SIZE);
    atomic_set(&_log_level, LOG_LEVEL_INFO);
    atomic_set(&_backup_number, DEFAULT_LOG_FILE_BACKUP_NUMBER);

    // 保证日志行最大长度不小于指定值
    _log_line_size = (log_line_size < LOG_LINE_SIZE_MIN)? LOG_LINE_SIZE_MIN: log_line_size;
}

CLogger::~CLogger()
{    
    //destroy(); 
    // 删除队列
    delete _log_queue;
    _log_queue = NULL;

    if (_log_fd != -1)
    {
        close(_log_fd);
        _log_fd = -1;      
    }
}

void CLogger::destroy()
{       
    { // _queue_lock
        LockHelper<CLock> lh(_queue_lock);

        // 停止Logger的日志，长度为0
        log_message_t* log_message = (log_message_t*)malloc(sizeof(log_message_t));
        log_message->length = 0;
        log_message->content[0] = '\0';
        _destroying = true;
        push_log_message(log_message);
    } // _queue_lock

    // 减引用计数，与create中的inc_refcount相呼应
    (void)dec_refcount();

    { // CLogger::_thread_lock
        LockHelper<CLock> lh(CLogger::_thread_lock);

        // 决定是否需要删除线程
        if (2 == CLogger::_log_thread->get_refcount())
        {
            CLogger::_log_thread->stop();
            CLogger::_log_thread->dec_refcount();
            CLogger::_log_thread = NULL;
        }        
    } // CLogger::_thread_lock
}

void CLogger::create(const char* log_path, const char* log_filename, uint32_t log_queue_size)
{
    // 日志文件路径和文件名
    snprintf(_log_path, sizeof(_log_path), "%s", log_path);
    snprintf(_log_filename, sizeof(_log_filename), "%s", log_filename);    

    // 创建日志队列
    uint32_t log_queue_size_ = log_queue_size;
    if (0 == log_queue_size_)
        log_queue_size_ = 1;
    _log_queue = new utils::CArrayQueue<log_message_t*>(log_queue_size_);
    
    // 创建和启动日志线程
    create_thread();

    // 创建日志文件
    inc_refcount(); // 和destroy一一对应
    create_logfile(false);
}

void CLogger::create_thread()
{
    LockHelper<CLock> lh(CLogger::_thread_lock);
    if (NULL == CLogger::_log_thread)
    {
        CLogger::_log_thread = new CLogThread;
        CLogger::_log_thread->inc_refcount();

        try
        {
            CLogger::_log_thread->start();
        }
        catch (CSyscallException& ex)
        {
            CLogger::_log_thread->dec_refcount();
            CLogger::_log_thread = NULL;
            throw; // 重新抛出异常
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool CLogger::execute()
{   
    // 增加引用计数，以保证下面的操作是安全的
    CLogger* self = const_cast<CLogger*>(this);
    CRefCountHelper<CLogger> rch(self);

    try
    {           
        // 写入前，预处理
        if (!prewrite())
        {
            return false;
        }
        if (_log_fd != -1)
        {
#if HAVE_UIO_H==1
            return batch_write();
#else
            return single_write();
#endif // HAVE_UIO_H         
        }        
    }
    catch (CSyscallException& ex )
    {
        fprintf(stderr, "Writed log %s/%s error: %s.\n", _log_path, _log_filename, ex.str().c_str());
        close_logfile();
    }

    return true;
}

bool CLogger::prewrite()
{
    if (need_create_file())
    {
        close_logfile();
        create_logfile(false);
    }
    else if (need_rotate_file())
    {
        close_logfile();
        rotate_file();
    }

    return is_registered();
}

bool CLogger::single_write()
{
    int retval;
    log_message_t* log_message = NULL;
    
    { // 限定锁的范围
        LockHelper<CLock> lh(_queue_lock);        
        log_message = _log_queue->pop_front();
        if (_waiter_number > 0)
            _queue_event.signal();        
    }
        
    // 分成两步，是避免write在Lock中
    if (log_message != NULL)
    {     
        // 读走信号
        read_signal(1);
        CLogger::_log_thread->dec_log_number(1);

        // 需要销毁Logger了
        if (0 == log_message->length)
        {
            free(log_message);
            return false;
        }

        // 循环处理中断
        for (;;)
        {
            retval = write(_log_fd, log_message->content, log_message->length);
            if ((-1 == retval) && (EINTR == Error::code()))
                continue;

            if (retval > 0)
            {
                _current_bytes += (uint32_t)retval;
            }
            
            break;
        }               

        // 释放消息
        free(log_message);                

        // 错误处理
        if (-1 == retval)
        {
            THROW_SYSCALL_EXCEPTION(NULL, errno, "write");
        }      
    }

    return true;
}

bool CLogger::batch_write()
{
    bool to_destroy_logger = false;

#if HAVE_UIO_H==1    
    int retval;
    int number = 0;
    struct iovec iov_array[LOG_NUMBER_WRITED_ONCE];
    
    { // 空括号用来限定_queue_lock的范围
        LockHelper<CLock> lh(_queue_lock);

        // 批量取出消息
        int i = 0;
        for (; i<LOG_NUMBER_WRITED_ONCE 
            && i<IOV_MAX
            && !_log_queue->is_empty(); ++i)
        {
            log_message_t* log_message = _log_queue->pop_front();                        
            if (log_message->length > 0)
            {                
                iov_array[i].iov_len = log_message->length;
                iov_array[i].iov_base = log_message->content; 
                ++number;
            }
            else
            {
                // 需要销毁Logger了
                read_signal(1);
                free(log_message);
                to_destroy_logger = true;
                CLogger::_log_thread->dec_log_number(1);
            }            
        }
        if (_waiter_number > 0)
        {
            _queue_event.broadcast();
        }
    }
  
    // 批量写入文件
    if (number > 0)
    {
        // 读走信号
        read_signal(number);
        CLogger::_log_thread->dec_log_number(number);

        // 循环处理中断
        for (;;)
        {
            retval = writev(_log_fd, iov_array, number);
            if ((-1 == retval) && (EINTR == Error::code()))
            {
                continue;
            }            
            if (retval > 0)
            {
                // 更新当前日志文件大小
                _current_bytes += static_cast<uint32_t>(retval);
            }
            
            break;
        }                

        // 释放消息
        while (number-- > 0)
        {
            log_message_t* log_message;
            void* iov_base = iov_array[number].iov_base;

            log_message = get_struct_head_address(log_message_t, content, iov_base);
            free(log_message);
        }

        // 错误处理
        if (-1 == retval)
        {
            throw CSyscallException(Error::code(), __FILE__, __LINE__, "logger writev");
        }
    }   
#endif // HAVE_UIO_H

    return !to_destroy_logger;
}

void CLogger::enable_screen(bool enabled)
{ 
    _screen_enabled = enabled;
}

void CLogger::enable_bin_log(bool enabled)
{    
    _bin_log_enabled = enabled;
}

void CLogger::enable_trace_log(bool enabled)
{ 
    _trace_log_enabled = enabled; 
}

void CLogger::enable_auto_adddot(bool enabled)
{
    _auto_adddot = enabled;
}

void CLogger::enable_auto_newline(bool enabled)
{ 
    _auto_newline = enabled;
}

void CLogger::set_log_level(log_level_t log_level)
{
    atomic_set(&_log_level, log_level);
}

void CLogger::set_single_filesize(uint32_t filesize)
{ 
    uint32_t max_bytes = (filesize < LOG_LINE_SIZE_MIN*10)? LOG_LINE_SIZE_MIN*10: filesize; 
    atomic_set(&_max_bytes, max_bytes);
}

void CLogger::set_backup_number(uint16_t backup_number) 
{
    atomic_set(&_backup_number, backup_number);
}

bool CLogger::enabled_bin()
{
    return _bin_log_enabled;
}

bool CLogger::enabled_detail()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_DETAIL;
}

bool CLogger::enabled_debug()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_DEBUG;
}

bool CLogger::enabled_info()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_INFO;
}

bool CLogger::enabled_warn()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_WARN;
}

bool CLogger::enabled_error()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_ERROR;
}

bool CLogger::enabled_fatal()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_FATAL;
}

bool CLogger::enabled_state()
{
    return atomic_read(&_log_level) <= LOG_LEVEL_STATE;
}

bool CLogger::enabled_trace()
{
    return _trace_log_enabled;
}

//////////////////////////////////////////////////////////////////////////

void CLogger::do_log(log_level_t log_level, const char* filename, int lineno, const char* module_name, const char* format, va_list& args)
{    
    va_list args_copy;
    va_copy(args_copy, args);
    utils::VaListHelper vh(args_copy);
    log_message_t* log_message = (log_message_t*)malloc(_log_line_size+sizeof(log_message_t)+1);

    char datetime[sizeof("2012-12-12 12:12:12/0123456789")];
    get_formatted_current_datetime(datetime, sizeof(datetime));
    
    // 模块名称
    std::string module_name_field;
    if (module_name != NULL)
    {
        module_name_field = std::string("[") + module_name + std::string("]");
    }

    // 在构造时，已经保证_log_line_size不会小于指定的值，所以下面的操作是安全的
    int head_length = utils::CStringUtils::fix_snprintf(
            log_message->content
          , _log_line_size
          , "[%s][0x%08x][%s]%s[%s:%d]"
          , datetime
          , CThread::get_current_thread_id()
          , get_log_level_name(log_level)
          , module_name_field.c_str()
          , filename
          , lineno);
    int log_line_length = vsnprintf(log_message->content+head_length, _log_line_size-head_length, format, args);

    if (log_line_length < _log_line_size-head_length)
    {
        log_message->length = head_length + log_line_length;
    }
    else
    {
        // 预定的缓冲区不够大，需要增大
        int new_line_length = log_line_length + head_length;
        if (new_line_length > LOG_LINE_SIZE_MAX)
            new_line_length = LOG_LINE_SIZE_MAX;
        
        log_message_t* new_log_message = (log_message_t*)malloc(new_line_length+sizeof(log_message_t)+1);
        if (NULL == new_log_message)
        {
            // 重新分配失败
            log_message->length = head_length + (log_line_length - 1);
        }
        else
        {
            free(log_message); // 释放老的，指向新的
            log_message = new_log_message;
                                    
            // 这里不需要关心返回值了
            head_length  = utils::CStringUtils::fix_snprintf(log_message->content, new_line_length, "[%s][0x%08x][%s]", datetime, CThread::get_current_thread_id(), get_log_level_name(log_level));
            log_line_length = utils::CStringUtils::fix_vsnprintf(log_message->content+head_length, new_line_length-head_length, format, args_copy);            
            log_message->length = head_length + log_line_length;
        }
    }
    
    // 自动添加结尾点号
    if (_auto_adddot 
     && (log_message->content[log_message->length-1] != '.')
     && (log_message->content[log_message->length-1] != '\n'))
    {
        log_message->content[log_message->length] = '.';
        log_message->content[log_message->length+1] = '\0';
        ++log_message->length;
    }

    // 自动添加换行符
    if (_auto_newline && (log_message->content[log_message->length-1] != '\n'))
    {
        log_message->content[log_message->length] = '\n';
        log_message->content[log_message->length+1] = '\0';
        ++log_message->length;
    }

    // 允许打屏
    if (_screen_enabled)
    {
        (void)write(STDOUT_FILENO, log_message->content, log_message->length);
    }
    
    // 日志消息放入队列中
    LockHelper<CLock> lh(_queue_lock);
    if (!_destroying)
    {
        push_log_message(log_message);
    }
}

void CLogger::push_log_message(log_message_t* log_message)
{    
    while (_log_queue->is_full())
    {
        ++_waiter_number;
        _queue_event.wait(_queue_lock);
        --_waiter_number;
    }

    CLogger::_log_thread->inc_log_number();
    _log_queue->push_back(log_message);    
    send_signal();
}

//////////////////////////////////////////////////////////////////////////

void CLogger::log_detail(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_detail())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);
        
        do_log(LOG_LEVEL_DETAIL, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_debug(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_debug())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_DEBUG, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_info(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_info())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_INFO, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_warn(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_warn())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_WARN, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_error(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_error())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_ERROR, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_fatal(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_fatal())
    {
        va_list args;        
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_FATAL, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_state(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_state())
    {
        va_list args;        
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_STATE, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_trace(const char* filename, int lineno, const char* module_name, const char* format, ...)
{         
    if (enabled_trace())
    {
        va_list args;
        va_start(args, format);
        utils::VaListHelper vh(args);

        do_log(LOG_LEVEL_TRACE, filename, lineno, module_name, format, args);
    }
}

void CLogger::log_bin(const char* filename, int lineno, const char* module_name, const char* log, uint16_t size)
{
    if (enabled_bin())
    {        
        //set_log_length(log, size);
        //_log_thread->push_log(log);
    }
}

//////////////////////////////////////////////////////////////////////////

void CLogger::close_logfile()
{
    // 关闭文件句柄
    if (_log_fd != -1)
    {        
        CLogger::_log_thread->remove_logger(this);
        close(_log_fd);
        _log_fd = -1;        
    }
}

void CLogger::create_logfile(bool truncate)
{    
    struct stat st;
    char filename[PATH_MAX+FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", _log_path, _log_filename);

    int flags = truncate? O_WRONLY|O_CREAT|O_TRUNC: O_WRONLY|O_CREAT|O_APPEND;
    _log_fd = open(filename, flags, FILE_DEFAULT_PERM);

    if (-1 == _log_fd)
    {
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");
    }    
    if (-1 == fstat(_log_fd, &st))
    {
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");
    }       
           
    _current_bytes = st.st_size;
    CLogger::_log_thread->register_logger(this);    
}

void CLogger::rotate_file()
{    
    int backup_number = atomic_read(&_backup_number);
    for (uint16_t i=backup_number; i>0; --i)
    {
        char old_filename[PATH_MAX+FILENAME_MAX];
        char new_filename[PATH_MAX+FILENAME_MAX];

        if (1 == i)
            (void)snprintf(old_filename, sizeof(old_filename), "%s/%s", _log_path, _log_filename);
        else     
            (void)snprintf(old_filename, sizeof(old_filename), "%s/%s.%d", _log_path, _log_filename, i-1);                    
        (void)snprintf(new_filename, sizeof(new_filename), "%s/%s.%d", _log_path, _log_filename, i);

        (void)rename(old_filename, new_filename);
    }

    create_logfile(0 == backup_number);
}

bool CLogger::need_create_file() const
{
    struct stat st;
    if (-1 == fstat(_log_fd, &st)) return false;

    return 0 == st.st_nlink;
}

//////////////////////////////////////////////////////////////////////////
CLogThread::CLogThread()  
    :_epoll_fd(-1)
{
    atomic_set(&_log_number, 0);
    _epoll_events = new struct epoll_event[LOGGER_NUMBER_MAX];
}

CLogThread::~CLogThread()
{    
    if (_epoll_fd != -1)
    {
        close(_epoll_fd);
    }

    delete []_epoll_events;
}

void CLogThread::run()
{
    // 提示
    fprintf(stderr, "[%s]Logger thread %u running.\n", CDatetimeUtils::get_current_datetime().c_str(), get_thread_id());

#if ENABLE_SET_LOG_THREAD_NAME==1
    CUtils::set_process_name("log-thread");
#endif // ENABLE_SET_LOG_THREAD_NAME

    // 所有日志都写完了，才可以退出日志线程
    while (!is_stop() || (get_log_number() > 0))
    {        
        int ret = epoll_wait(_epoll_fd, _epoll_events, LOGGER_NUMBER_MAX, -1);
        if (-1 == ret)
        {
            if (EINTR == Error::code()) continue;
            THROW_SYSCALL_EXCEPTION(NULL, errno, "epoll_wait");
        }

        for (int i=0; i<ret; ++i)
        {
            CLogProber* log_prober = static_cast<CLogProber*>(_epoll_events[i].data.ptr);
            if (!log_prober->execute())
            {
                // LogProber要么为Logger，要么为LogThread
                // ，但只有Logger，才会进入这里，所以强制转换成立
                CLogger* logger = static_cast<CLogger*>(log_prober);
                remove_logger(logger);                
            }
        }
    }

    // 提示
    fprintf(stderr, "[%s]Logger thread %u exited.\n", CDatetimeUtils::get_current_datetime().c_str(), get_thread_id());
    remove_object(this);
}

void CLogThread::before_stop() throw (utils::CException, CSyscallException)
{
    send_signal();
}

void CLogThread::before_start() throw (utils::CException, CSyscallException)
{
    // 创建Epoll
    _epoll_fd = epoll_create(LOGGER_NUMBER_MAX);
    if (-1 == _epoll_fd)
    {
        fprintf(stderr, "Logger created epoll error: %s.\n", Error::to_string().c_str());
        THROW_SYSCALL_EXCEPTION(NULL, errno, "epoll_create");
    }

    // 将pipe放入epoll中
    register_object(this);
}

bool CLogThread::execute()
{
    read_signal(1);
    return true;
}

void CLogThread::remove_logger(CLogger* logger)
{
    try
    {
        if (logger->is_registered())
        {
            remove_object(logger);
            logger->set_registered(false);
            logger->dec_refcount();
        }
    }
    catch (CSyscallException& ex)
    {
        // TODO
    }
}

void CLogThread::register_logger(CLogger* logger)
{
    try
    {
        logger->inc_refcount();
        register_object(logger);
        logger->set_registered(true);
    }
    catch (CSyscallException& ex)
    {
        logger->dec_refcount();
        throw;
    }
}

void CLogThread::remove_object(CLogProber* log_prober)
{
    if (-1 == epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, log_prober->get_fd(), NULL))
    {
        THROW_SYSCALL_EXCEPTION(NULL, errno, "epoll_ctl");
    }
}

void CLogThread::register_object(CLogProber* log_prober)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = log_prober;

    if (-1 == epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, log_prober->get_fd(), &event))
    {
        THROW_SYSCALL_EXCEPTION(NULL, errno, "epoll_ctl");
    }
}

SYS_NAMESPACE_END
