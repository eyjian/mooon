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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com
 */
#include "utils/string_utils.h"
#include "utils/scoped_ptr.h"
#include "utils/tokener.h"
//#include <alloca.h>
#include <ctype.h> // toupper
#include <limits>
#include <stdarg.h>
UTILS_NAMESPACE_BEGIN

/***
  * 快速字符串转换成整数模板通用函数
  * @str: 需要被转换的字符串
  * @result: 存储转换后的结果
  * @max_length: 该整数类型对应的字符串的最多字符个数，不包括结尾符
  * @converted_length: 需要转换的字符串长度，如果为0则表示转换整个字符串
  * @ignored_zero: 是否忽略开头的0
  * @return: 如果转换成功返回true, 否则返回false
  */
template <typename IntType>
static bool fast_string2int(const char* str, IntType& result, uint8_t max_length, uint8_t converted_length, bool ignored_zero)
{
    bool negative = false;
    const char* tmp_str = str;
    if (NULL == str) return false;

    // 处理负数
    if ('-' == tmp_str[0])
    {
        // 负数
        negative = true;
        ++tmp_str;
    }

    // 处理空字符串
    if ('\0' == tmp_str[0]) return false;

    // 处理0打头的
    if ('0' == tmp_str[0])
    {
        // 如果是0开头，则只能有一位数字
        if (('\0' == tmp_str[1]) || (1 == converted_length))
        {
            result = 0;
            return true;
        }
        else
        {
            if (!ignored_zero) return false;
            for (;;)
            {
                ++tmp_str;
                if (tmp_str - str > max_length-1) return false;
                if (*tmp_str != '0') break;
            }
            if ('\0' == *tmp_str)
            {
                result = 0;
                return true;
            }
        }
    }

    // 检查第一个字符
    if ((*tmp_str < '0') || (*tmp_str > '9')) return false;
    result = (*tmp_str - '0');

    while ((0 == converted_length) || (tmp_str - str < converted_length-1))
    {
        ++tmp_str;
        if ('\0' == *tmp_str) break;
        if (tmp_str - str > max_length-1) return false;

        if ((*tmp_str < '0') || (*tmp_str > '9')) return false;

        result = result * 10;
        result += (*tmp_str - '0');
    }

    if (negative)
        result = -result;
    return true;
}

std::string& CStringUtils::reverse_string(std::string* str)
{
    std::string& str_ref = *str;

    for (std::string::size_type i=0; i<str_ref.size()/2; ++i)
    {
        const std::string::size_type left = i;
        const std::string::size_type right = str_ref.size() - i - 1;
        const char lelf_c = str_ref[left];
        const char right_c = str_ref[right];
        str_ref[left] = right_c;
        str_ref[right] = lelf_c;
    }

    return str_ref;
}

std::string CStringUtils::reverse_string(const std::string& str)
{
    std::string reversed_str = str;
    reverse_string(&reversed_str);
    return reversed_str;
}

std::string& CStringUtils::remove_last(std::string& source, char c)
{
    std::string::size_type pos = source.rfind(c);
    if (pos+1 != source.length())
        source.erase(pos);
    return source;
}

std::string CStringUtils::remove_last(const std::string& source, char c)
{
    std::string str = source;
    return remove_last(str, c);
}

std::string& CStringUtils::remove_last(std::string& source, const std::string& sep)
{
    // std: $HOME/bin/exe
    // sep: /bin/
    // ---> $HOME
    std::string::size_type pos = source.rfind(sep);
    if (pos != std::string::npos)
        source.erase(pos);
    return source;
}

std::string CStringUtils::remove_last(const std::string& source, const std::string& sep)
{
    std::string str = source;
    return remove_last(str, sep);
}

char* CStringUtils::to_upper(char* source)
{
    char* tmp_source = source;
    while (*tmp_source != '\0')
    {
        if ((*tmp_source >= 'a') && (*tmp_source <= 'z'))
            *tmp_source += 'A' - 'a';

        ++tmp_source;
    }

    return source;
}

char* CStringUtils::to_lower(char* source)
{
    char* tmp_source = source;
    while (*tmp_source != '\0')
    {
        if ((*tmp_source >= 'A') && (*tmp_source <= 'Z'))
            *tmp_source += 'a' - 'A';

        ++tmp_source;
    }

    return source;
}

std::string& CStringUtils::to_upper(std::string& source)
{
    // 只修改大小写，可以这样做
    char* tmp_source = (char *)source.c_str();
    to_upper(tmp_source);
    return source;
}

std::string CStringUtils::to_upper(const std::string& source)
{
    std::string str = source;
    return to_upper(str);
}

std::string& CStringUtils::to_lower(std::string& source)
{
    // 只修改大小写，可以这样做
    char* tmp_source = (char *)source.c_str();
    to_lower(tmp_source);
    return source;
}

std::string CStringUtils::to_lower(const std::string& source)
{
    std::string str = source;
    return to_lower(str);
}

/** 判断指定字符是否为空格或TAB符(\t)或回车符(\r)或换行符(\n) */
bool CStringUtils::is_space(char c)
{
    return (' ' == c) || ('\t' == c) || ('\r' == c) || ('\n' == c);
}

// 不使用trim_left和trim_right组合实现，以保持效率
void CStringUtils::trim(char* source)
{
    char* space = NULL;
    char* tmp_source = source;
    while (' ' == *tmp_source) ++tmp_source;

    for (;;)
    {
        *source = *tmp_source;
        if ('\0' == *tmp_source)
        {
            if (space != NULL)
                *space = '\0';
            break;
        }
        else if (is_space(*tmp_source))
        {
            if (NULL == space)
                space = source;
        }
        else
        {
            space = NULL;
        }

        ++source;
        ++tmp_source;
    }
}

void CStringUtils::trim_left(char* source)
{
    char* tmp_source = source;
    while (is_space(*tmp_source)) ++tmp_source;

    for (;;)
    {
        *source = *tmp_source;
        if ('\0' == *tmp_source) break;

        ++source;
        ++tmp_source;
    }
}

void CStringUtils::trim_right(char* source)
{
    char* space = NULL;
    char* tmp_source = source;

    for (;;)
    {
        if ('\0' == *tmp_source)
        {
            if (space != NULL)
                *space = '\0';
            break;
        }
        else if (is_space(*tmp_source))
        {
            if (NULL == space)
                space = tmp_source;
        }
        else
        {
            space = NULL;
        }

        ++tmp_source;
    }
}

std::string& CStringUtils::trim(std::string& source)
{
    trim_left(source);
    trim_right(source);
    return source;
}

std::string CStringUtils::trim(const std::string& source)
{
    std::string str = source;
    trim_left(str);
    trim_right(str);
    return str;
}

std::string& CStringUtils::trim_left(std::string& source)
{
    // 不能直接对c_str()进行修改，因为长度发生了变化
    size_t length = source.length();
    char* tmp_source = new char[length+1];
    DeleteHelper<char> dh(tmp_source, true);

    strncpy(tmp_source, source.c_str(), length);
    tmp_source[length] = '\0';

    trim_left(tmp_source);
    source = tmp_source;
    return source;
}

std::string CStringUtils::trim_left(const std::string& source)
{
    std::string str = source;
    trim_left(str);
    return str;
}

std::string& CStringUtils::trim_right(std::string& source)
{
    // 不能直接对c_str()进行修改，因为长度发生了变化
    size_t length = source.length();
    char* tmp_source = new char[length+1];
    DeleteHelper<char> dh(tmp_source, true);

    strncpy(tmp_source, source.c_str(), length);
    tmp_source[length] = '\0';

    trim_right(tmp_source);
    source = tmp_source;
    return source;
}

std::string CStringUtils::trim_right(const std::string& source)
{
    std::string str = source;
    trim_right(str);
    return str;
}

bool CStringUtils::string2double(const char* source, double& result)
{
    char* endptr = NULL;
    double result_ = strtod(source, &endptr);
    if ((endptr != NULL) && ('\0' == *endptr))
        result = result_;
    return ((endptr != NULL) && ('\0' == *endptr));
}

double CStringUtils::string2double(const char* source)
{
    return strtod(source, NULL);
}

bool CStringUtils::string2float(const char* source, float& result)
{
    char* endptr = NULL;
    float result_ = strtof(source, &endptr);
    if ((endptr != NULL) && ('\0' == *endptr))
        result = result_;
    return ((endptr != NULL) && ('\0' == *endptr));
}

float CStringUtils::string2float(const char* source)
{
    return strtof(source, NULL);
}

bool CStringUtils::string2ldouble(const char* source, long double& result)
{
    char* endptr = NULL;
    long double result_ = strtold(source, &endptr);
    if ((endptr != NULL) && ('\0' == *endptr))
        result = result_;
    return ((endptr != NULL) && ('\0' == *endptr));
}

long double CStringUtils::string2ldouble(const char* source)
{
    return strtold(source, NULL);
}

bool CStringUtils::string2int8(const char* source, int8_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, int8_t& result, uint8_t converted_length, bool ignored_zero)
{
    int16_t value = 0;

    if (!string2int16(source, value, converted_length, ignored_zero)) return false;
    if (value < std::numeric_limits<int8_t>::min() 
     || value > std::numeric_limits<int8_t>::max()) return false;
    
    result = (int8_t)value;
    return true;
}

bool CStringUtils::string2int16(const char* source, int16_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, int16_t& result, uint8_t converted_length, bool ignored_zero)
{
    int32_t value = 0;

    if (!string2int32(source, value, converted_length, ignored_zero)) return false;
    if (value < std::numeric_limits<int16_t>::min()
     || value > std::numeric_limits<int16_t>::max()) return false;

    result = (int16_t)value;
    return true;
}

bool CStringUtils::string2int32(const char* source, int32_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, int32_t& result, uint8_t converted_length, bool ignored_zero)
{
    if (NULL == source) return false;

    long value;
    if (!fast_string2int<long>(source, value, sizeof("-2147483648")-1, converted_length, ignored_zero)) return false;
    if ((value < std::numeric_limits<int32_t>::min())
     || (value > std::numeric_limits<int32_t>::max()))  return false;

    result = (int32_t)value;
    return true;
}

bool CStringUtils::string2int64(const char* source, int64_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, int64_t& result, uint8_t converted_length, bool ignored_zero)
{
    long long value;
    if (!fast_string2int<long long>(source, value, sizeof("-9223372036854775808")-1, converted_length, ignored_zero)) return false;

    result = (int64_t)value;
    return true;
}

bool CStringUtils::string2time_t(const char* source, time_t& result, uint8_t converted_length, bool ignored_zero)
{
    // time_t是一个有符号类型，
    // 32位time_t可能是long的别名，不同于int，虽然字节数可能相同
    if (sizeof(time_t) == sizeof(int64_t))
    {
        int64_t result64;
        if (!string2int64(source, result64, converted_length, ignored_zero))
            return false;
        result = static_cast<time_t>(result64);
    }
    else
    {
        int32_t result32;
        if (!string2int32(source, result32, converted_length, ignored_zero))
            return false;
        result = static_cast<time_t>(result32);
    }

    return true;
}

bool CStringUtils::string2size_t(const char* source, size_t& result, uint8_t converted_length, bool ignored_zero)
{
    // size_t是一个无符号类型，
    // 32位size_t可能是unsigned long的别名，不同于unsigned int，虽然字节数可能相同
    if (sizeof(size_t) == sizeof(uint64_t))
    {
        uint64_t result64;
        if (!string2uint64(source, result64, converted_length, ignored_zero))
            return false;
        result = static_cast<size_t>(result64);
    }
    else
    {
        uint32_t result32;
        if (!string2uint32(source, result32, converted_length, ignored_zero))
            return false;
        result = static_cast<size_t>(result32);
    }

    return true;
}

bool CStringUtils::string2uint8(const char* source, uint8_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, uint8_t& result, uint8_t converted_length, bool ignored_zero)
{
    uint16_t value = 0;
    if (!string2uint16(source, value, converted_length, ignored_zero)) return false;
    if (value > std::numeric_limits<uint8_t>::max()) return false;

    result = (uint8_t)value;
    return true;
}

bool CStringUtils::string2uint16(const char* source, uint16_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, uint16_t& result, uint8_t converted_length, bool ignored_zero)
{
    uint32_t value = 0;
    if (!string2uint32(source, value, converted_length, ignored_zero)) return false;
    if (value > std::numeric_limits<uint16_t>::max()) return false;

    result = (uint16_t)value;
    return true;
}

bool CStringUtils::string2uint32(const char* source, uint32_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, uint32_t& result, uint8_t converted_length, bool ignored_zero)
{
    unsigned long value;
    if (!fast_string2int<unsigned long>(source, value, sizeof("4294967295")-1, converted_length, ignored_zero)) return false;

    result = (uint32_t)value;
    return true;
}

bool CStringUtils::string2uint64(const char* source, uint64_t& result, uint8_t converted_length, bool ignored_zero)
{
    return string2int(source, result, converted_length, ignored_zero);
}

bool CStringUtils::string2int(const char* source, uint64_t& result, uint8_t converted_length, bool ignored_zero)
{
    unsigned long long value;
    if (!fast_string2int<unsigned long long>(source, value, sizeof("18446744073709551615")-1, converted_length, ignored_zero)) return false;

    result = (uint64_t)value;
    return true;
}

std::string CStringUtils::int16_tostring(int16_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(int16_t source)
{
    // # define UINT16_MAX             (65535)
    // # define INT16_MAX              (32767)
    char str[sizeof("065535")]; // 0xFFFF
    snprintf(str, sizeof(str), "%d", (int)source);
    return str;
}

std::string CStringUtils::int32_tostring(int32_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(int32_t source)
{
    // # define UINT32_MAX             (4294967295U)
    // # define INT32_MAX              (2147483647)
    char str[sizeof("04294967295")]; // 0xFFFFFFFF
    snprintf(str, sizeof(str), "%d", source);
    return str;
}

std::string CStringUtils::int64_tostring(int64_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(int64_t source)
{
    char str[sizeof("018446744073709551615")]; // 0xFFFFFFFFFFFFFFFF
    snprintf(str, sizeof(str), "%" PRId64, source);
//#if __WORDSIZE==64
//    snprintf(str, sizeof(str), "%ld", source);
//#else
//    snprintf(str, sizeof(str), "%lld", source);
//#endif
    return str;
}

std::string CStringUtils::uint16_tostring(uint16_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(uint16_t source)
{
    // # define INT16_MAX              (32767)
    // # define UINT16_MAX             (65535)
    char str[sizeof("065535")]; // 0xFFFF
    snprintf(str, sizeof(str), "%d", (int)source);
    return str;
}

std::string CStringUtils::uint32_tostring(uint32_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(uint32_t source)
{
    // # define UINT32_MAX             (4294967295U)
    char str[sizeof("04294967295")]; // 0xFFFFFFFF
    snprintf(str, sizeof(str), "%u", source);
    return str;
}

std::string CStringUtils::uint64_tostring(uint64_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(uint64_t source)
{
    // # define UINT64_MAX             (__UINT64_C(18446744073709551615))
    char str[sizeof("018446744073709551615")]; // 0xFFFFFFFFFFFFFFFF
    snprintf(str, sizeof(str), "%" PRId64, source);

//#if __WORDSIZE==64
//    snprintf(str, sizeof(str), "%lu", source);
//#else
//    snprintf(str, sizeof(str), "%llu", source);
//#endif
    return str;
}

char* CStringUtils::skip_spaces(char* buffer)
{
    char* iter = buffer;
    while (' ' == *iter) ++iter;

    return iter;
}

const char* CStringUtils::skip_spaces(const char* buffer)
{
    const char* iter = buffer;
    while (' ' == *iter) ++iter;

    return iter;
}

uint32_t CStringUtils::hash(const char *str, int len)
{
    uint32_t g;
    uint32_t h = 0;
    const char *p = str;

    while (p < str+len)
    {
        h = (h << 4) + *p++;
        if ((g = (h & 0xF0000000)))
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h;
}

// snprintf()第2个参数的大小，要求包含结尾符'\0'
// snprintf()的返回值，返回的是期望大小，但不包含结尾符'\0'，下面假设snprintf()的第二个参数值为10，则：
// 1) 当str为"abc"时，它的返回值的大小是3，"abc"的字符个数刚好是3；
// 2) 当str为"1234567890"时，它的返回值大小是10，"1234567890"的字符个数刚好是10；
// 3) 当str为"1234567890X"时，它的返回值大小是11，"1234567890X"的字符个数刚好是11。
int CStringUtils::fix_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int expected = fix_vsnprintf(str, size, format, ap);
    va_end(ap);

    return expected;
}

int CStringUtils::fix_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int expected = vsnprintf(str, size, format, ap);

    if (expected < static_cast<int>(size))
        return expected + 1;

    return static_cast<int>(size);
}

std::string CStringUtils::path2filename(const std::string& path, const std::string& join_string)
{
    std::string filename;
    std::list<std::string> tokens;
    CTokener::split(&tokens, path, "/");

    if (!tokens.empty())
    {
        filename = tokens.front();
        tokens.pop_front();
    }

    while (!tokens.empty())
    {
        filename += join_string + tokens.front();
        tokens.pop_front();
    }

    return filename;
}

int CStringUtils::chr_index(const char* str, char c) 
{
    const char* c_position = strchr(str, c);
    return (NULL == c_position)? -1: c_position-str;
}

int CStringUtils::chr_rindex(const char* str, char c) 
{
    const char* c_position = strrchr(str, c);
    return (NULL == c_position)? -1: c_position-str;
}

std::string CStringUtils::extract_dirpath(const char* filepath)
{
    std::string dirpath;
    int index = chr_rindex(filepath, '/');
    if (index != -1)    
        dirpath.assign(filepath, index);    

    return dirpath;
}

// 提取示例（printf("%s => %s\n", str.c_str(), extract_filename(str).c_str());）：
// /a/b/c => c
// abc => abc
// /abc => abc
// abc.cpp => abc.cpp
std::string CStringUtils::extract_filename(const std::string& filepath)
{
    std::string filename;
    const char* slash_position = strrchr(filepath.c_str(), '/');
    if (NULL == slash_position)
        filename = filepath;
    else
        filename.assign(slash_position+1);

    return filename;
}

/**
 * 提取示例：
/a/b/c => c
abc => abc
/abc => abc
abc.txt => abc.txt
. => .
 =>
 */
const char* CStringUtils::extract_filename(const char* filepath)
{
    const char* slash_position = strrchr(filepath, '/');
    if (NULL == slash_position)
        return filepath;

    return slash_position + 1;
}

// snprintf()第2个参数的大小，要求包含结尾符'\0'
// snprintf()的返回值，返回的是期望大小，但不包含结尾符'\0'，下面假设snprintf()的第二个参数值为10，则：
// 1) 当str为"abc"时，它的返回值的大小是3，"abc"的字符个数刚好是3；
// 2) 当str为"1234567890"时，它的返回值大小是10，"1234567890"的字符个数刚好是10；
// 3) 当str为"1234567890X"时，它的返回值大小是11，"1234567890X"的字符个数刚好是11。
std::string CStringUtils::format_string(const char* format, ...)
{
    va_list ap;
    size_t size = 8192;
    ScopedArray<char> buffer(new char[size]);

    while (true)
    {
        va_start(ap, format);

        // vsnprintf中的第二参数大小是要求包含结尾符的
        int expected = vsnprintf(buffer.get(), size, format, ap);

        va_end(ap);
        if (expected > -1 && expected < (int)size)
            break;

        /* Else try again with more space. */
        if (expected > -1)    /* glibc 2.1 */
            size = (size_t)expected + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        buffer.reset(new char[size]);
    }

    return buffer.get();
}

bool CStringUtils::is_numeric_string(const char* str, bool enable_float)
{
    const char* p = str;
    bool found_dot = false;

    while (*p != '\0')
    {
        if (!(*p >= '0' && *p <= '9'))
        {
            if (*p != '.')
            {
                return false;
            }
            else if (!enable_float)
            {
                // 不允许小数
                return false;
            }
            else // 可能是小数
            {
                if (found_dot) // 小数只会有一个点
                    return false;
                if (p == str) // 小数点不能为第一个字符，即不允许：“.2017”这样的小数
                    return false;

                found_dot = true;
            }
        }

        ++p;
    }

    return true;
}

bool CStringUtils::is_alphabetic_string(const char* str)
{
    const char* p = str;

    while (*p != '\0')
    {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')))
            return false;

        ++p;
    }

    return true;
}

bool CStringUtils::is_variable_string(const char* str)
{
    const char* p = str;

    while (*p != '\0')
    {
        if (*p >= '0' && *p <= '9')
        {
            ++p;
            continue;
        }
        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
        {
            ++p;
            continue;
        }
        if (('_' == *p) || ('-' == *p))
        {
            ++p;
            continue;
        }

        return false;
    }

    return true;
}

bool CStringUtils::is_regex_string(const char* str)
{
    const char* p = str;

    while (*p != '\0')
    {
        if (*p >= '0' && *p <= '9')
        {
            ++p;
            continue;
        }
        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
        {
            ++p;
            continue;
        }
        if (('_' == *p) || ('-' == *p))
        {
            ++p;
            continue;
        }
        if (('\\' == *p) || ('/' == *p))
        {
            ++p;
            continue;
        }
        if (('+' == *p) || ('*' == *p))
        {
            ++p;
            continue;
        }
        if (('.' == *p) || (',' == *p))
        {
            ++p;
            continue;
        }
        if (('<' == *p) || ('>' == *p))
        {
            ++p;
            continue;
        }
        if (('{' == *p) || ('}' == *p))
        {
            ++p;
            continue;
        }
        if (('(' == *p) || (')' == *p))
        {
            ++p;
            continue;
        }
        if (('[' == *p) || (']' == *p))
        {
            ++p;
            continue;
        }
        if (('$' == *p) || ('%' == *p) || ('^' == *p) || ('?' == *p) || ('"' == *p) || (' ' == *p))
        {
            ++p;
            continue;
        }

        return false;
    }

    return true;
}

std::string CStringUtils::remove_suffix(const std::string& filename)
{
    std::string::size_type pos = filename.find('.');
    if (pos == std::string::npos)
    {
        return filename;
    }
    else
    {
        return filename.substr(0, pos);
    }
}

std::string CStringUtils::replace_suffix(const std::string& filepath, const std::string& new_suffix)
{
    std::string::size_type pos = filepath.find('.');
    if (pos == std::string::npos)
    {
        if (new_suffix.empty() || new_suffix == ".")
        {
            return filepath;
        }
        else
        {
            if ('.' == new_suffix[0])
                return filepath + new_suffix;
            else
                return filepath + std::string(".") + new_suffix;
        }
    }
    else
    {
        if (new_suffix.empty() || new_suffix == ".")
        {
            // 相当于删除了后缀
            return filepath.substr(0, pos);
        }
        else
        {
            if ('.' == new_suffix[0])
                return filepath.substr(0, pos) + new_suffix;
            else
                return filepath.substr(0, pos) + std::string(".") + new_suffix;
        }
    }
}

std::string CStringUtils::to_hex(const std::string& source, bool lowercase)
{
    std::string hex;
    hex.resize(source.size()*2);

    char* hex_p = const_cast<char*>(hex.data());
    for (std::string::size_type i=0; i<source.size(); ++i)
    {
        if (lowercase)
            snprintf(hex_p, 3, "%02x", source[i]);
        else
            snprintf(hex_p, 3, "%02X", source[i]);

        hex_p += 2;
    }

    *hex_p = '\0';
    return hex;
}

// 一个汉字，
// GBK编码时占2字节，UTF8编码时占3字节
std::string CStringUtils::encode_url(const std::string& url, bool space2plus)
{
    return encode_url(url.c_str(), url.size(), space2plus);
}

std::string CStringUtils::encode_url(const char* url, size_t url_length, bool space2plus)
{
    static char hex[] = "0123456789ABCDEF";
    std::string result(url_length*3+1, '\0');

    int i = 0;
    while (*url != '\0')
    {
        char c = *url++;

        if (' ' == c)
        {
            if (space2plus)
            {
                result[i++] = '+';
            }
            else
            {
                // 新标准将空格替换为加号+
                result[i+0] = '%';
                result[i+1] = '2';
                result[i+2] = '0';
                i += 3;
            }
        }
        else if ((c >= '0' && c <= '9') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 (c == '-') || (c == '_') ||
                 (c == '.') || (c == '~'))
        {
            // RFC 3986标准定义的未保留字符 (2005年1月)
            result[i++] = c;
        }
        else
        {
            // 有符号的c值可能是负数
            result[i+0] = '%';
            result[i+1] = hex[static_cast<unsigned char>(c) / 16];
            result[i+2] = hex[static_cast<unsigned char>(c) % 16];
            i += 3;
        }
    }

    result.resize(i);
    return result;
}

std::string CStringUtils::decode_url(const std::string& encoded_url)
{
    return decode_url(encoded_url.c_str(), encoded_url.size());
}

std::string CStringUtils::decode_url(const char* encoded_url, size_t encoded_url_length)
{
    std::string result(encoded_url_length+1, '\0');

    int i = 0;
    while (*encoded_url != '\0')
    {
        char c = *encoded_url++;

        if (c == '+')
        {
            result[i++] = ' ';
        }
        else if (c != '%')
        {
            result[i++] = c;
        }
        else
        {
            if (!isxdigit(encoded_url[0]) ||
                !isxdigit(encoded_url[1]))
            {
                result[i++] = '%';
            }
            else
            {
                char hex[3];
                hex[0] = encoded_url[0];
                hex[1] = encoded_url[1];
                hex[2] = '\0';

                char x = strtol(hex, NULL, 16);
                result[i++] = x;
                encoded_url += 2;
            }
        }
    }

    result.resize(i);
    return result;
}

// CR: Carriage Return
// LF: Line Feed
void CStringUtils::trim_CR(char* line)
{
    if (line != NULL)
    {
        size_t len = strlen(line);
        if ('\r' == line[len-1])
            line[len-1] = '\0';
    }
}

void CStringUtils::trim_CR(std::string& line)
{
    std::string::size_type tail = line.size() - 1;
    if ('\r' == line[tail])
        line.resize(tail);
}

std::string CStringUtils::char2hex(unsigned char c)
{
    static unsigned char hex_table[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    std::string hex(2, '\0');

    hex[0] = hex_table[(c >> 4) & 0x0F];
    hex[1] = hex_table[c & 0x0F];
    return hex;
}

unsigned char CStringUtils::hex2char(const std::string& hex)
{
    unsigned char c = 0;

    for (int i=0; i<std::min<int>(hex.size(), 2); ++i)
    {
        unsigned char c1 = toupper(hex[i]); // ctype.h
        unsigned char c2 = (c1 >= 'A') ? (c1 - ('A' - 10)) : (c1 - '0');
        (c <<= 4) += c2;
    }

    return c;
}

const std::string& CStringUtils::replace_string(const char* src, std::string* dest, const std::vector<std::pair<char, std::string> >& rules)
{
    for (int i=0; src[i]!='\0'; ++i)
    {
        bool have_replaced = false;

        for (std::vector<std::pair<char, std::string> >::size_type j=0; j<rules.size(); ++j)
        {
            if (src[i] == rules[j].first)
            {
                dest->append(rules[j].second);
                have_replaced = true;
                break;
            }
        }

        if (!have_replaced)
        {
            dest->push_back(src[i]);
        }
    }

    return *dest;
}

const std::string& CStringUtils::replace_string(const std::string& src, std::string* dest, const std::vector<std::pair<char, std::string> >& rules)
{
    return replace_string(src.c_str(), dest, rules);
}

void CStringUtils::parse_filename(const std::string& filename, std::string* shortname, std::string* suffix)
{
    const std::string::size_type dot_pos = filename.rfind('.');

    if (dot_pos == std::string::npos)
    {
        shortname->clear();
        suffix->clear();
    }
    else
    {
        shortname->assign(filename.c_str(), dot_pos);
        suffix->assign(filename.c_str() + dot_pos + 1);
    }
}

bool CStringUtils::nodeV4_from_str(const std::string& node, std::string* ip, uint16_t* port)
{
    const std::string::size_type pos = node.find(":");

    if (pos == std::string::npos)
    {
        return false;
    }
    else
    {
        *ip = node.substr(0, pos);
        return (ip->size() >= 7) && (ip->size() <= 15) && // 1.1.1.1, 111.111.111.111
                mooon::utils::CStringUtils::string2int(node.substr(pos+1).c_str(), *port);
    }
}

UTILS_NAMESPACE_END
