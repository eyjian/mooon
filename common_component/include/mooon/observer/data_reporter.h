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
 */
#ifndef MOOON_OBSERVER_DATA_REPORTER_H
#define MOOON_OBSERVER_DATA_REPORTER_H
#include <mooon/observer/config.h>
#include <stdarg.h>
#include <mooon/sys/log.h>
#include <mooon/utils/scoped_ptr.h>
#include <mooon/utils/string_utils.h>
OBSERVER_NAMESPACE_BEGIN

/***
  * 上报或记录日志
  */
class CALLBACK_INTERFACE IDataReporter
{
public:
    /** 虚拟析构函数，仅为应付编译器告警 */
    virtual ~IDataReporter() {}

    /** 上报或记录日志 */
	virtual void report(const char* format, ...) {}

	/** 上报或记录日志 */
    virtual void report(const void* data, uint32_t data_size) {}
};

/**
 * 默认的，能满足多数时候的需求
 */
class CDefaultDataReporter: public IDataReporter
{
public:
    // line_length 第行数据的最大长度
    CDefaultDataReporter(mooon::sys::ILogger* report_logger, size_t line_length=1024)
        : _report_logger(report_logger), _line_length(line_length)
    {
    }

private:
    virtual void report(const char* format, ...)
    {
        va_list ap;
        utils::ScopedArray<char> line(new char[_line_length+1]);

        va_start(ap, format);
        utils::CStringUtils::fix_vsnprintf(line.get(), _line_length+1, format, ap);
        _report_logger->log_raw("%s", line.get());
        va_end(ap);
    }

private:
    mooon::sys::ILogger* _report_logger;
    size_t _line_length;
};

OBSERVER_NAMESPACE_END
#endif // MOOON_OBSERVER_DATA_REPORTER_H
