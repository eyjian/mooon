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
#ifndef MOOON_UTILS_STRING_FORMATTER_H
#define MOOON_UTILS_STRING_FORMATTER_H
#include "utils/config.h"
UTILS_NAMESPACE_BEGIN

/***
 * StringFormatter解决什么问题？看下列一段代码：
class CException
{
public:
    explicit CException(int errcode, const char* errmsg);
};

void foo()
{
    throw CException(20141216, "invalid param");
}

这里的 "invalid param" 不够具体，foo()的调用者看到的是非完整的信息
，如果调用者会将errmsg记录到日志，由于信息有损，会增加定位问题的难度。

进行如下改进：
void foo()
{
    std::stringstream ss;
    ss < "param[" << param_name << "] invalid: " << param_value;
    throw CException(20141216, ss.str());
}

上面的代码解决了问题，但书写有些繁琐。借助StringFormatter，则变成：
void foo()
{
    throw CException(20141216, StringFormatter("param[%s] invalid: %s", param_name, param_value));
}

当然，这种做法同printf()一样，存在类型安全问题。
 */

/**
 * 将变参转换成一个对象，使用示例:
void foo(const StringFormatter& formatter)
{
    printf("%s\n", formatter.c_str());
}

extern "C" int main()
{
    foo(StringFormatter("%s|%d", "abc", 123));
    return 0;
}
*/
class StringFormatter
{
public:
    // 使用简单，固化buffer_size
    explicit StringFormatter(const char* format, ...) throw (std::bad_alloc) __attribute__((format(printf, 2, 3)))
        : _buffer_size(1024)
    {
        va_list ap;
        va_start(ap, format);

        init(format, ap);
        va_end(ap);
    }

    // 增加灵活性，buffer_size由参数传入
    explicit StringFormatter(size_t buffer_size, const char* format, ...) throw (std::bad_alloc) __attribute__((format(printf, 3, 4)))
        : _buffer_size(buffer_size)
    {
        va_list ap;
        va_start(ap, format);

        init(format, ap);
        va_end(ap);
    }

    StringFormatter(const StringFormatter& other)
        : _buffer(other.str()), _buffer_size(other.buffer_size())
    {        
    }

    size_t buffer_size() const throw ()
    {
        return _buffer_size;
    }

    // 仿std::string::c_str()命名
    const char* c_str() const throw ()
    {
        return _buffer.c_str();
    }

    // 仿std::stringstream::str()命名
    const std::string& str() const throw ()
    {
        return _buffer;
    }

private:
    void init(const char* format, va_list& ap) throw (std::bad_alloc)
    {        
        char* buffer = new char[_buffer_size];
        vsnprintf(buffer, _buffer_size, format, ap);

        // 保存结果
        DeleteHelper<char> dh(buffer, true);
        _buffer = buffer;
    }

private:
    StringFormatter();    
    StringFormatter& operator =(const StringFormatter&);
    bool operator ==(const StringFormatter&) const;
    bool operator !=(const StringFormatter&) const;

private:
    std::string _buffer;
    size_t _buffer_size;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_STRING_FORMATTER_H
