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
#ifndef MOOON_UTILS_EXCEPTION_H
#define MOOON_UTILS_EXCEPTION_H
#include "mooon/utils/string_formatter.h"
#include <exception>
#include <string>

// 抛出CException异常
#define THROW_EXCEPTION(errmsg, errcode) \
    throw ::mooon::utils::CException(errmsg, errcode, __FILE__, __LINE__)

// 抛出用户定制异常
#define THROW_CUSTOM_EXCEPTION(errmsg, errcode, exception_class) \
    throw exception_class(errmsg, errcode, __FILE__, __LINE__)

UTILS_NAMESPACE_BEGIN

// 异常基类，继承自标准库的exception
class CException: public std::exception
{
public:
    // errmsg 错误信息
    // errcode 错误代码
    // file 发生错误的源代码文件
    // line 发生错误的代码行号
    CException(const char* errmsg, int errcode, const char* file, int line) throw ();
    CException(const std::string& errmsg, int errcode, const std::string& file, int line) throw ();
    virtual ~CException() throw () {}

    virtual const char* what() const throw ();
    const char* file() const throw ();
    int line() const throw ();
    int errcode() const throw ();

    // 返回一个可读的字符串
    virtual std::string str() const throw ();

private:
    virtual std::string prefix() const throw ();

private:
    void init(const char* errmsg, int errcode, const char* file, int line) throw ();

protected:
    std::string _errmsg;
    int _errcode;
    std::string _file;
    int _line;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_EXCEPTION_H
