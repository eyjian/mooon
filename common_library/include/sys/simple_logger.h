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
 * 简单的写日志类，非线程安全，提供按大小滚动功能
 * 不追求功能，也不追求性能，只求简单，若要功能强、性能高，可以使用CLogger
 *
 * 使用方法：
 * 1) 构造一个CSimpleLogger对象
 *    CSimpleLogger logger(".", "test.log", 1024*1024, 10);
 * 2) 调用print方法写日志
 *    logger.print("%s\n", "test");
 */
#ifndef MOOON_SYS_SIMPLE_LOGGER_H
#define MOOON_SYS_SIMPLE_LOGGER_H

// 只要定义了NOT_WITH_MOOON宏，
// 则本文件和mooon无任何关系，方便集成到自己的代码中
//#define NOT_WITH_MOOON
#if !defined(NOT_WITH_MOOON)
#include <sys/config.h>
#endif // NOT_WITH_MOOON

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sstream>
#if !defined(NOT_WITH_MOOON)
SYS_NAMESPACE_BEGIN
#endif // NOT_WITH_MOOON

/***
  * 万能型类型转换函数
  */
template <typename AnyType>
inline std::string any2string(AnyType any_value)
{
    std::stringstream result_stream;
    result_stream << any_value;

    return result_stream.str();
}

/***
  * 取当前时间，和date_util.h有重复，但为保持simple_logger.h的独立性，在所难免
  */
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
    CSimpleLogger(const std::string& log_dir
                 ,const std::string& filename
                 ,unsigned int log_size = 1024*1024*100
                 ,unsigned char log_numer = 10
                 ,unsigned short record_size = 8192);
    ~CSimpleLogger();

    /** 日志文件是否创建或打开成功 */
    bool is_ok() const;
    
    /** 输出日志，象printf一样使用，不自动加换行符 */
    void print(const char* format, va_list& ap);
    void print(const char* format, ...);

    /** 刷新日志，因为使用FILE是带缓存的 */
    void flush();

private:
    void reset();    /** 复位状态值 */
    void rotate_log(); /** 滚动日志 */

private:
    FILE* _fp;                    /** 当前正在写的日志文件描述符 */
    char* _log_buffer;            /** 存放日志的Buffer */
    int _bytes_writed;            /** 已经写入的字节数 */
    std::string _log_dir;         /** 日志存放目录 */
    std::string _filename;        /** 日志文件名，不包含目录部分 */
    unsigned int _log_size;       /** 单个日志文件的大小 */
    unsigned char _log_numer;     /** 日志滚动的个数 */
    unsigned short _record_size;  /** 单条日志的大小，单位为字节数 */
};

inline CSimpleLogger::CSimpleLogger(
                     const std::string& log_dir 
                    ,const std::string& filename
                    ,unsigned int log_size
                    ,unsigned char log_numer
                    ,unsigned short record_size)
 :_fp(NULL)
 ,_log_buffer(NULL)
 ,_bytes_writed(0)
 ,_log_dir(log_dir)
 ,_filename(filename)
 ,_log_size(log_size)
 ,_log_numer(log_numer)
 ,_record_size(record_size)
{
    std::string log_path = log_dir + std::string("/") + filename;
    _fp = fopen(log_path.c_str(), "a");
    
    if (_fp != NULL)
    {
        if (-1 == fseek(_fp, 0, SEEK_END))
        {
            // 失败，将不会写日志
            fclose(_fp);
            _fp = NULL;
        }
        else
        {
            // 取得已有大小
            _bytes_writed = ftell(_fp);

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
            
            _log_buffer = new char[_record_size];
        }
    }
}

inline CSimpleLogger::~CSimpleLogger()
{
    if (_fp != NULL)
        fclose(_fp);
        
    delete []_log_buffer;
}

inline bool CSimpleLogger::is_ok() const
{
    return _fp != NULL;
}

inline void CSimpleLogger::print(const char* format, va_list& ap)
{
    if (_fp != NULL)
    {
        char datetime_buffer[sizeof("2012-12-21 00:00:00")]; // 刚好世界末日
        get_current_datetime(datetime_buffer, sizeof(datetime_buffer));

        vsnprintf(_log_buffer, _record_size, format, ap);
        int bytes_writed = fprintf(_fp, "[%s]%s", datetime_buffer, _log_buffer);
        if (bytes_writed > 0)
            _bytes_writed += bytes_writed;

        if (_bytes_writed > static_cast<int>(_log_size))
        {
            rotate_log();
        }
    }
}

inline void CSimpleLogger::print(const char* format, ...)
{
    if (_fp != NULL)
    {
        va_list ap;
        va_start(ap, format);
        
        print(format, ap);
        va_end(ap);
    }
}

inline void CSimpleLogger::rotate_log()
{
    std::string new_path; // 滚动后的文件路径，包含目录和文件名
    std::string old_path; // 滚动前的文件路径，包含目录和文件名
    
    reset(); // 轮回，一切重新开始
    
    // 历史滚动
    for (int i=_log_numer-1; i>0; --i)
    {
        new_path = _log_dir + std::string("/") + _filename + std::string(".") + any2string(i);
        old_path = _log_dir + std::string("/") + _filename + std::string(".") + any2string(i-1);

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
    _fp = fopen(old_path.c_str(), "w+");
}

inline void CSimpleLogger::reset()
{
    _bytes_writed = 0;

    if (_fp != NULL)
    {
        fclose(_fp);
        _fp = NULL;
    }
}

inline void CSimpleLogger::flush()
{
    if (_fp != NULL)
        fflush(_fp);
}

/***
  * 测试代码
#include "simple_logger.h"
int main()
{
    CSimpleLogger logger(".", "test.log", 10240);
    for (int i=0; i<100000; ++i)
        logger.print("%d ==> abcdefghijklmnopqrestuvwxyz.\n", i);
        
    return 0;
}
*/

#if !defined(NOT_WITH_MOOON)
SYS_NAMESPACE_END
#endif // NOT_WITH_MOOON
#endif // MOOON_SYS_SIMPLE_LOGGER_H
