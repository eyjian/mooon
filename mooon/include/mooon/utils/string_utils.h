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
#ifndef MOOON_UTILS_STRING_UTILS_H
#define MOOON_UTILS_STRING_UTILS_H
#include "mooon/utils/config.h"
#include <math.h>
#include <sstream>
#include <vector>
UTILS_NAMESPACE_BEGIN

class CStringUtils
{
public:
    // 反转字符串
    // 输入空则啥也不做，输入单个字符则啥也不做，
    // 输入12则变成21，输入123则变成321，输入1234则变成4321，依次类推。。。
    // str 即是输入参数，也是输出参数，作为输入参数时存储被反转的字符串，作为输出参数存储反转后的字符串
    // 返回值指向反转后的字符串
    static std::string& reverse_string(std::string* str);

    // 反转字符串
    // str 被反转的字符串
    // 返回值为反转后的字符串
    static std::string reverse_string(const std::string& str);

    /** 删除字符串尾部从指定字符开始的内容
      * @source: 需要处理的字符串
      * @c: 分隔字符
      * @example:  如果str为“/usr/local/test/bin/”，而c为“/”，
      *            则处理后str变成“/usr/local/test/bin”
      */
    static std::string& remove_last(std::string& source, char c);
    static std::string remove_last(const std::string& source, char c);

    /** 删除字符串尾部从指定字符串开始的内容
      * @source: 需要处理的字符串
      * @sep: 分隔字符串
      * @example: 如果str为“/usr/local/test/bin/tt”，而sep为“/bin/”，
      *           则处理后str变成“/usr/local/test
      */
    static std::string& remove_last(std::string& source, const std::string& sep);
    static std::string remove_last(const std::string& source, const std::string& sep);

    /** 将字符串中的所有小写字符转换成大写 */
    static char* to_upper(char* source);
    static std::string& to_upper(std::string& source);
    static std::string to_upper(const std::string& source);

    /** 将字符串中的所有大写字符转换成小写 */
    static char* to_lower(char* source);
    static std::string& to_lower(std::string& source);
    static std::string to_lower(const std::string& source);

    /** 判断指定字符是否为空格或TAB符(\t)或回车符(\r)或换行符(\n) */
    static bool is_space(char c);
    
    /** 删除字符串首尾空格或TAB符(\t)或回车符(\r)或换行符(\n) */
    static void trim(char* source);
    static std::string& trim(std::string& source);
    static std::string trim(const std::string& source);

    /** 删除字符串首部空格或TAB符(\t)或回车符(\r)或换行符(\n) */
    static void trim_left(char* source);
    static std::string& trim_left(std::string& source);
    static std::string trim_left(const std::string& source);

    /** 删除字符串尾部空格或TAB符(\t)或回车符(\r)或换行符(\n) */
    static void trim_right(char* source);        
    static std::string& trim_right(std::string& source);
    static std::string trim_right(const std::string& source);

    /**
     * 字符串转双精度浮点类型
     * 如果source是“1.23”则转换成功返回true，如果source是“1.2a”则转换失败返回false
     */
    static bool string2double(const char* source, double& result);
    static double string2double(const char* source);

    /** 字符串转浮点类型 */
    static bool string2float(const char* source, float& result);
    static float string2float(const char* source);

    /** 字符串转长双精度类型 */
    static bool string2ldouble(const char* source, long double& result);
    static long double string2ldouble(const char* source);

    /**
      * 字符串转换成整数函数
      */
	
    /** 将字符串转换成8位的有符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, int8_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2int8(const char* source, int8_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    /** 将字符串转换成16位的有符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, int16_t& result, uint8_t converted_length=0, bool ignored_zero=false);
	static bool string2int16(const char* source, int16_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    /** 将字符串转换成32位的有符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, int32_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2int32(const char* source, int32_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    /** 将字符串转换成64位的有符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, int64_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2int64(const char* source, int64_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2time_t(const char* source, time_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2size_t(const char* source, size_t& result, uint8_t converted_length=0, bool ignored_zero=false);

    /** 将字符串转换成8位的无符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, uint8_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2uint8(const char* source, uint8_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    /** 将字符串转换成16位的无符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, uint16_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2uint16(const char* source, uint16_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    /** 将字符串转换成32位的无符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, uint32_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2uint32(const char* source, uint32_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    /** 将字符串转换成64位的无符号整数
      * @source: 待转换成整数的字符串
      * @result: 转换后的整数
      * @converted_length: 需要进行数字转换的字符个数，超过部分不做解析，如果取值为0则处理整个字符串
      * @ignored_zero: 是否允许允许字符串以0打头，并自动忽略
      * @return: 如果转换成功返回true，否则返回false
      */
    static bool string2int(const char* source, uint64_t& result, uint8_t converted_length=0, bool ignored_zero=false);
    static bool string2uint64(const char* source, uint64_t& result, uint8_t converted_length=0, bool ignored_zero=false);    

    // 当str为非有效的IntType类型整数值时，返回error_value指定的值
    template <typename IntType>
    static IntType string2int(const std::string& str, IntType error_value=0)
    {
        IntType m = 0;
        uint8_t converted_length = 0;
        bool ignored_zero = false;
        if (!string2int(str.c_str(), m, converted_length, ignored_zero))
            m = error_value;

        return m;
    }

    static std::string int_tostring(int16_t source);
    static std::string int16_tostring(int16_t source);    

    static std::string int32_tostring(int32_t source);
    static std::string int_tostring(int32_t source);

    static std::string int_tostring(int64_t source);
    static std::string int64_tostring(int64_t source);

    static std::string int_tostring(uint16_t source);
    static std::string uint16_tostring(uint16_t source);    

    static std::string int_tostring(uint32_t source);
    static std::string uint32_tostring(uint32_t source);    

    static std::string int_tostring(uint64_t source);
    static std::string uint64_tostring(uint64_t source);    
    
    /** 跳过空格部分
      */
    static char* skip_spaces(char* buffer);
    static const char* skip_spaces(const char* buffer);

    static uint32_t hash(const char *str, int len);

    /***
      * 不同于标准库的snprintf，这里的snprintf总是保证返回实际向str写入的字节数，
      * 包括结尾符，而不管size是否足够容纳，其它行为相同
      */
    static int fix_snprintf(char *str, size_t size, const char *format, ...) __attribute__((format(printf, 3, 4)));
    static int fix_vsnprintf(char *str, size_t size, const char *format, va_list ap);

    /** 路径转换成文件名 */
    static std::string path2filename(const std::string& path, const std::string& join_string);

    /** 万用类型转换函数 */
    template <typename Any>
    static std::string any2string(Any any)
    {
        std::stringstream s;
        s << any;
        return s.str();
    }

    /** 将STL容器转换成字符串 */
    template <class ContainerClass>
    static std::string container2string(const ContainerClass& container, const std::string& join_string)
    {
        std::string str;
        typename ContainerClass::const_iterator iter = container.begin();

        for (; iter!=container.end(); ++iter)
        {
            if (str.empty())
                str = any2string(*iter);
            else
                str += join_string + any2string(*iter);
        }

        return str;
    }
    
    /** 将map容器转换成字符串 */
    template <class MapClass>
    static std::string map2string(const MapClass& map, const std::string& join_string)
    {
        std::string str;
        typename MapClass::const_iterator iter = map.begin();

        for (; iter!=map.end(); ++iter)
        {
            if (str.empty())
                str = any2string(iter->second);
            else
                str += join_string + any2string(iter->second);
        }

        return str;
    }

    /***
      * 求得一个字符在字符串中的位置
      * @return 如果c在字符串中，则返回非负整数值，否则返回-1
      */
    static int chr_index(const char* str, char c);
    static int chr_rindex(const char* str, char c);

    /***
      * 从文件路径中提取目录路径
      * @return 返回提取到的目录路径
      */
    static std::string extract_dirpath(const char* filepath);
    
    /***
      * 从文件路径中提取出文件名
      * @return 返回提取到文件名
      *         如果输入是空则返回空，如果输入是.则返回.，如果输入是/则返回空
      * 提取示例：
      * /a/b/c => c
      * abc => abc
      * /abc => abc
      * abc.cpp => abc.cpp
      * / =>
      *   =>
      * . => .
      */
    static std::string extract_filename(const std::string& filepath);

    // 返回filepath上文件名的起始位置
    /**
     * 提取示例：
    /a/b/c => c
    abc => abc
    /abc => abc
    abc.txt => abc.txt
    . => .
     =>
     */
    static const char* extract_filename(const char* filepath);

    /***
      * 通过格式化生成一个字符串
      * @return 返回生成的字符串
      * 注意，如果过长，会导致占用过多的内存，甚至程序崩溃
      */
    static std::string format_string(const char* format, ...) __attribute__((format(printf, 1, 2)));

    // 判断一个字符串是否为纯数字型字符串，包括小数（小数点不能是第一个字符）
    static bool is_numeric_string(const char* str, bool enable_float=true);

    // 判断一个字符串是否为纯字母型字符串，而不包含数字、下划线等非字母
    static bool is_alphabetic_string(const char* str);

    // 判断一个字符串是否为变量型字符串，即：可包含字母、数字、下划线和横线
    static bool is_variable_string(const char* str);
    
    // 判断一个字符串是否可能为正则表达式字符串
    static bool is_regex_string(const char* str);

    // 删除文件名的后缀部分，如abc.exe变成abc，注意不会处理路径部分
    static std::string remove_suffix(const std::string& filename);
    
    // 替换或删除后缀，效果如下：
    // filepath new_suffix 返回值
    // abc      log        abc.log
    // abc      .log       abc.log
    // abc      .          abc
    // abc                 abc
    // abc.exe  log        abc.log
    // abc.exe  .log       abc.log
    // abc.exe  .          abc
    // abc.exe             abc
    static std::string replace_suffix(const std::string& filepath, const std::string& new_suffix);

    // 将一个字符流转换成十六进制字符串
    // source 注意它是一个字节流，不一定是字符串
    // 返回十六进制字符串
    static std::string to_hex(const std::string& source, bool lowercase=true);

    // URL编码
    // space2plus 如果为true则空格被编码成“加号”，否则被编码成“%20”
    static std::string encode_url(const std::string& url, bool space2plus=false);
    static std::string encode_url(const char* url, size_t url_length, bool space2plus=false);

    // URL解码
    static std::string decode_url(const std::string& encoded_url);
    static std::string decode_url(const char* encoded_url, size_t encoded_url_length);

    // 删除行尾的回车符
    static void trim_CR(char* line);
    static void trim_CR(std::string& line);

    static std::string char2hex(unsigned char c);
    static unsigned char hex2char(const std::string& hex);

    // 扫描源字符串src，进行替换
    // 返回值指向dest的字符串，dest替换后的完整字符串
    // 注意src和dst不能为同一对象
    //
    // 假如src为azb，rules是<a,12>、<b,345>，则dest值为12z345
    //
    // 使用示例：
    // int main()
    // {
    //     std::string str, dest;
    //     std::vector<std::pair<char, std::string> > rules(2);
    //     rules[0] = std::make_pair('<', "&lt;");
    //     rules[1] = std::make_pair('>', "&gt;");
    //
    //     str = "<12>";
    //     printf("%s\n", replace_string(str, &dest, rules).c_str());
    //
    //     return 0;
    // }
    //
    // 运行输出：
    // &lt;12&gt;
    static const std::string& replace_string(const char* src, std::string* dest, const std::vector<std::pair<char, std::string> >& rules);
    static const std::string& replace_string(const std::string& src, std::string* dest, const std::vector<std::pair<char, std::string> >& rules);

    // filename 包含后缀的文件名，如：test.json
    // shortname 不包含后缀的文件名，如：test
    // suffix 文件名的后缀，如：json
    //
    // 如果filename是标准的shortname.suffix格式，
    // 则返回后shortname存储shortname值，suffix存储suffix值
    // 但如果filename不带后缀，或者以点号结尾，则返回后shortname和suffix值均为空
    static void parse_filename(const std::string& filename, std::string* shortname, std::string* suffix);

    // 将一个格式为“ip:port”的字符串转换成IP和端口
    // 如果不符合此格式返回false，或者端口不在有效范围内的数字也返回false
    // 对ip只做了弱检查，即长度要求满足IPV4的地址长度，没做更严格的检查。
    static bool nodeV4_from_str(const std::string& node, std::string* ip, uint16_t* port);
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_STRING_UTILS_H
