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
#include "utils/tokener.h"
//#include <alloca.h>
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

void CStringUtils::remove_last(std::string& source, char c)
{
    std::string::size_type pos = source.rfind(c);
    if (pos+1 != source.length())
        source.erase(pos);
}

void CStringUtils::remove_last(std::string& source, const std::string& sep)
{
    // std: $HOME/bin/exe
    // sep: /bin/
    // ---> $HOME
    std::string::size_type pos = source.rfind(sep);
    if (pos != std::string::npos)
        source.erase(pos);
}

void CStringUtils::to_upper(char* source)
{
    char* tmp_source = source;
    while (*tmp_source != '\0')
    {
        if ((*tmp_source >= 'a') && (*tmp_source <= 'z'))
            *tmp_source += 'A' - 'a';

        ++tmp_source;
    }
}

void CStringUtils::to_lower(char* source)
{
    char* tmp_source = source;
    while (*tmp_source != '\0')
    {
        if ((*tmp_source >= 'A') && (*tmp_source <= 'Z'))
            *tmp_source += 'a' - 'A';

        ++tmp_source;
    }
}

void CStringUtils::to_upper(std::string& source)
{
    // 只修改大小写，可以这样做
    char* tmp_source = (char *)source.c_str();
    to_upper(tmp_source);
}

void CStringUtils::to_lower(std::string& source)
{
    // 只修改大小写，可以这样做
    char* tmp_source = (char *)source.c_str();
    to_lower(tmp_source);
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

void CStringUtils::trim(std::string& source)
{
    trim_left(source);
    trim_right(source);
}

void CStringUtils::trim_left(std::string& source)
{
    // 不能直接对c_str()进行修改，因为长度发生了变化
    size_t length = source.length();
    char* tmp_source = new char[length+1];
    DeleteHelper<char> dh(tmp_source, true);

    strncpy(tmp_source, source.c_str(), length);
    tmp_source[length] = '\0';

    trim_left(tmp_source);
    source = tmp_source;
}

void CStringUtils::trim_right(std::string& source)
{
    // 不能直接对c_str()进行修改，因为长度发生了变化
    size_t length = source.length();
    char* tmp_source = new char[length+1];
    DeleteHelper<char> dh(tmp_source, true);

    strncpy(tmp_source, source.c_str(), length);
    tmp_source[length] = '\0';

    trim_right(tmp_source);
    source = tmp_source;
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
    char str[sizeof("065535")]; // 0xFFFF
    snprintf(str, sizeof(str), "%d", source);
    return str;
}

std::string CStringUtils::int32_tostring(int32_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(int32_t source)
{
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
    snprintf(str, sizeof(str), "%"PRId64, source);
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
    char str[sizeof("065535")]; // 0xFFFF
    snprintf(str, sizeof(str), "%u", source);
    return str;
}

std::string CStringUtils::uint32_tostring(uint32_t source)
{
    return int_tostring(source);
}

std::string CStringUtils::int_tostring(uint32_t source)
{
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
    char str[sizeof("018446744073709551615")]; // 0xFFFFFFFFFFFFFFFF
#if __WORDSIZE==64
    snprintf(str, sizeof(str), "%lu", source);
#else
    snprintf(str, sizeof(str), "%llu", source);
#endif
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

// snprintf()第2个参数的大小，要求包含结尾符'\0'
// snprintf()的返回值，返回的是期望大小，但不包含结尾符'\0'，下面假设snprintf()的第二个参数值为10，则：
// 1) 当str为"abc"时，它的返回值的大小是3，"abc"的字符个数刚好是3；
// 2) 当str为"1234567890"时，它的返回值大小是10，"1234567890"的字符个数刚好是10；
// 3) 当str为"1234567890X"时，它的返回值大小是11，"1234567890X"的字符个数刚好是11。
// 最多支持10240个ANSI字符，超过的会被截断，但调用者可能不清楚是否发生了截断@_@
std::string CStringUtils::format_string(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    // size不包含结尾符，所以在分配内存时需要加一
    size_t size = 1024;
    char* buffer = new char[size + 1];

    // vsnprintf中的第二参数大小是要求包含结尾符的
    int expected = vsnprintf(buffer, size + 1, format, ap);
    if (expected >= ((int)size+1))
    {
        // 防止太长，撑死内存
        if (expected > 10240)
            expected = 10240;

        // expected的大小不包含结尾符，所以在分配内存时需要加一
        delete []buffer;
        buffer = new char[expected + 1];

        va_end(ap);
        va_start(ap, format);

        vsnprintf(buffer, static_cast<size_t>(expected + 1), format, ap);
    }

    va_end(ap);
    DeleteHelper<char> dh(buffer, true);
    return buffer;
}

bool CStringUtils::is_numeric_string(const char* str)
{
    const char* p = str;

    while (*p != '\0')
    {
        if (!(*p >= '0' && *p <= '9'))
            return false;

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

UTILS_NAMESPACE_END
