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
 * Writed by yijian on 2015/7/18, eyjian@gmail/eyjian@qq.com
 */
#ifndef MOOON_UTILS_ARGS_PARSER_H
#define MOOON_UTILS_ARGS_PARSER_H
#include <mooon/utils/any2string.h>
#include <mooon/utils/string_utils.h>
#include <map>
#include <vector>

// 使用示例：
//
// #include <mooon/utils/args_parser.h>
//
// STRING_ARG_DEFINE(ip, "127.0.0.1", "listen IP address");
// INTEGER_ARG_DEFINE(uint16_t, port, 2015, 1000, 5000, "listen port");
// DATE_STRING_ARG_DEFINE(startdate, "2017-09-05", "start date, format: YYYY-MM-DD", true);
//
// int main(int argc, char* argv[])
// {
//     std::string errmsg;
//     if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
//     {
//         fprintf(stderr, "%s\n", errmsg.c_str());
//         fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
//         exit(1);
//     }
//
//     printf("ip: %s\n", mooon::argument::ip->c_value());
//     printf("port: %u\n", mooon::argument::port->value());
//     return 0;
// }

// 应当总是在main()函数所在文件中调用STRING_ARG_DEFINE()和INTEGER_ARG_DEFINE()，
// 如果其它文件中也需要访问，则应当使用STRING_ARG_DECLARE()和INTEGER_ARG_DECLARE()。

// 注意不用要在其它namespace内调用
// 字符串类型参数定义（供main()函数所在文件中调用）
// param_name 参数名
// default_value 参数的默认值，如果没有通过命令行指定，则default_value为参数值
// help_string 对这个参数的说明
//
// 使用示例（假设参数名为ip）：
// STRING_ARG_DEFINE(ip, "127.0.0.1", "listen IP address");
// mooon::argument::ip->value(); // 返回类型为“const std::string”
// mooon::argument::ip->c_value(); // 返回类型为“const char*”
#define STRING_ARG_DEFINE(param_name, default_value, help_string) \
    namespace mooon { namespace argument \
    { \
        utils::CStringArgument* param_name = \
            new utils::CStringArgument( \
                #param_name, default_value, help_string); \
    }}

// 日期格式的参数定义，要求值格式为：YYYY-MM-DD
// enable_empty 是否允许空字符串值，比如：--startdate=""，如果enable_empty为true则允许，为false则不允许
#define DATE_STRING_ARG_DEFINE(param_name, default_value, help_string, enable_empty) \
    namespace mooon { namespace argument \
    { \
        utils::CDateStringArgument* param_name = \
            new utils::CDateStringArgument( \
                #param_name, default_value, help_string, enable_empty); \
    }}

// 时间格式的参数定义，要求值格式为：hh:mm:ss
#define TIME_STRING_ARG_DEFINE(param_name, default_value, help_string, enable_empty) \
    namespace mooon { namespace argument \
    { \
        utils::CTimeStringArgument* param_name = \
            new utils::CTimeStringArgument( \
                #param_name, default_value, help_string, enable_empty); \
    }}

// 日期时间格式的参数定义，要求值格式为：YYYY-MM-DD hh:mm:ss
#define DATETIME_STRING_ARG_DEFINE(param_name, default_value, help_string, enable_empty) \
    namespace mooon { namespace argument \
    { \
        utils::CDatetimeStringArgument* param_name = \
            new utils::CDatetimeStringArgument( \
                #param_name, default_value, help_string, enable_empty); \
    }}

// 布尔类型参数定义，值只能为false或true，只能为小写
// 注意default_value值必须为false或true，只能为小写
#define BOOL_STRING_ARG_DEFINE(param_name, default_value, help_string) \
    namespace mooon { namespace argument \
    { \
        utils::CBoolStringArgument* param_name = \
            new utils::CBoolStringArgument( \
                #param_name, default_value, help_string); \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数定义（供main()函数所在文件中调用）
//
// 如遇到如下所示的编译错误，
// 这是因为int_type指定了不带长度的类型，如time_t或long等，指定为带长度的类型，如int32_t等即可解决
// no matching function for call to 'mooon::utils::CStringUtils::string2int(const char*, long int&, uint8_t&, bool&)'
//
// int_type 参数的数据类型
// param_name 参数名
// default_value 参数的默认值，如果没有通过命令行指定，则default_value为参数值
// help_string 对这个参数的说明
// min_value 可取的最小值
// max_value 可取的最大值
//
// 使用示例（假设参数名为port）:
// INTEGER_ARG_DEFINE(uint16_t, port, 2015, 1000, 65535, "listen port");
// mooon::argument::port->value(); // 返回类型为“uint16_t”
#define INTEGER_ARG_DEFINE(int_type, param_name, default_value, min_value, max_value, help_string) \
    namespace mooon { namespace argument \
    { \
        utils::CIntArgument<int_type>* param_name = \
            new utils::CIntArgument<int_type>( \
                #param_name, default_value, min_value, max_value, help_string); \
    }}

// 定义双精度类型参数
#define DOUBLE_ARG_DEFINE(param_name, default_value, min_value, max_value, help_string) \
    namespace mooon { namespace argument \
    { \
        utils::CDoubleArgument* param_name = \
            new utils::CDoubleArgument( \
                #param_name, default_value, min_value, max_value, help_string); \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数声明（供非main()函数所在文件中调用）
#define INTEGER_ARG_DECLARE(int_type, param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CIntArgument<int_type>* param_name; \
    }}

 // 声明双精度类型参数
#define DOUBLE_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CDoubleArgument* param_name; \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数声明（供非main()函数所在文件中调用）
#define STRING_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CStringArgument* param_name; \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数声明（供非main()函数所在文件中调用）
#define DATE_STRING_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CDateStringArgument* param_name; \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数声明（供非main()函数所在文件中调用）
#define TIME_STRING_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CTimeStringArgument* param_name; \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数声明（供非main()函数所在文件中调用）
#define DATETIME_STRING_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CDatetimeStringArgument* param_name; \
    }}

// 注意不用要在其它namespace内调用
// 整数类型参数声明（供非main()函数所在文件中调用）
#define BOOL_STRING_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern utils::CBoolStringArgument* param_name; \
    }}

////////////////////////////////////////////////////////////////////////////////
UTILS_NAMESPACE_BEGIN

extern std::string g_help_string; // --help的说明
extern std::string g_version_string; // --version的说明

// 解析命令行参数
// 如果解析出错，则返回false，errmsg保存出错原因
bool parse_arguments(int argc, char* argv[], std::string* errmsg);

////////////////////////////////////////////////////////////////////////////////
class CArgumentBase
{
public:
    CArgumentBase(const std::string& name, const std::string& help_string);
    virtual ~CArgumentBase() {}
    virtual bool check_value(std::string* errmsg) { set_checked(); return true; }
    virtual bool set_value(const std::string& new_value, std::string* errmsg) = 0;
    virtual std::string usage_string() const { return std::string(""); }
    virtual bool is_true() const { return true; }
    virtual bool is_false() const { return false; }

public:
    bool checked() const
    {
        return _checked;
    }

    const std::string& name() const
    {
        return _name;
    }

    const std::string& help_string() const
    {
        return _help_string;
    }

    const char* c_name() const
    {
        return _name.c_str();
    }

    const char* c_help_string() const
    {
        return _help_string.c_str();
    }

    void set_checked()
    {
        _checked = true;
    }

private:
    bool _checked; // 参数是否被检查过
    std::string _name;
    std::string _help_string;
};

class CArgumentContainer
{
    SINGLETON_DECLARE(CArgumentContainer);

public:
    ~CArgumentContainer();
    void add_argument(CArgumentBase* argument);
    bool set_argument(const std::string& name, const std::string& value, std::string* errmsg);
    std::string usage_string() const;
    bool check_parameters(std::string* errmsg) const;

private:
    typedef std::map<std::string, CArgumentBase*> ArgumentTable;
    ArgumentTable _argument_table;
    typedef std::vector<CArgumentBase*> ArgumentList;
    ArgumentList _argument_list;
};

////////////////////////////////////////////////////////////////////////////////
class CStringArgument: public CArgumentBase
{
public:
    CStringArgument(const std::string& name, const std::string& default_value, const std::string& help_string, bool enable_empty=true)
        : CArgumentBase(name, help_string), _enable_empty(enable_empty), _default_value(default_value), _value(default_value)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

    const std::string& default_value() const
    {
        return _default_value;
    }

    const std::string& value() const
    {
        return _value;
    }

    const char* c_default_value() const
    {
        return _default_value.c_str();
    }

    const char* c_value() const
    {
        return _value.c_str();
    }

    bool enable_empty() const
    {
        return _enable_empty;
    }

public:
    virtual bool is_true() const
    {
        return !_value.empty();
    }

    virtual bool is_false() const
    {
        return _value.empty();
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        _value = new_value;
        return true;
    }

    virtual std::string usage_string() const
    {
        std::string prefix;
        if (name().length() < 2)
            prefix = "-";
        else
            prefix = "--";

        return mooon::utils::CStringUtils::format_string(
             "%s%s[%s]: %s", prefix.c_str(), c_name(), c_default_value(), c_help_string());
    }

protected:
    bool handle_empty_value(std::string* errmsg) const
    {
        if (enable_empty())
        {
            return true;
        }
        else
        {
            if (name().size() < 2)
            {
                *errmsg = CStringUtils::format_string("parameter[-%s] not set", name().c_str());
            }
            else
            {
                *errmsg = CStringUtils::format_string("parameter[--%s] not set", name().c_str());
            }

            return false;
        }
    }

protected:
    bool _enable_empty;
    std::string _default_value;
    std::string _value;
};

// 要求格式为：YYYY-MM-DD
class CDateStringArgument: public CStringArgument
{
public:
    CDateStringArgument(const std::string& name, const std::string& default_value, const std::string& help_string, bool enable_empty)
        : CStringArgument(name, default_value, help_string, enable_empty)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

private:
    bool check_value(const std::string& value, std::string* errmsg)
    {
        set_checked();

        if (value.empty())
        {
            return handle_empty_value(errmsg);
        }

        do
        {
            if (value.size() != sizeof("YYYY-MM-DD")-1)
            {
                break;
            }
            if ((value[4] != '-') ||
                (value[7] != '-'))
            {
                break;
            }

            uint16_t year, month, day;
            const std::string& year_str = value.substr(0, 4);
            const std::string& month_str = value.substr(5, 2);
            const std::string& day_str = value.substr(8, 2);
            if (!CStringUtils::string2int(year_str.c_str(), year, 0, false))
            {
                break;
            }
            if (!CStringUtils::string2int(month_str.c_str(), month, 0, true))
            {
                break;
            }
            if (!CStringUtils::string2int(day_str.c_str(), day, 0, true))
            {
                break;
            }
            if (year < 1970)
            {
                break;
            }
            if (month > 12)
            {
                break;
            }
            if (day > 31)
            {
                break;
            }
            if (2==month && day>29)
            {
                break;
            }

            return true;
        } while(false);

        if (name().size() < 2)
            *errmsg = CStringUtils::format_string("parameter[-%s] with invalid date format (YYYY-MM-DD): %s", name().c_str(), value.c_str());
        else
            *errmsg = CStringUtils::format_string("parameter[--%s] with invalid date format (YYYY-MM-DD): %s", name().c_str(), value.c_str());
        return false;
    }

    virtual bool check_value(std::string* errmsg)
    {
        return check_value(_value, errmsg);
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        if (!check_value(new_value, errmsg))
        {
            return false;
        }
        else
        {
            _value = new_value;
            return true;
        }
    }
};

// 要求格式为：hh:mm:ss
class CTimeStringArgument: public CStringArgument
{
public:
    CTimeStringArgument(const std::string& name, const std::string& default_value, const std::string& help_string, bool enable_empty)
        : CStringArgument(name, default_value, help_string, enable_empty)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

private:
    bool check_value(const std::string& value, std::string* errmsg)
    {
        set_checked();

        if (value.empty())
        {
            return handle_empty_value(errmsg);
        }

        do
        {
            if (value.size() != sizeof("hh:mm:ss")-1)
            {
                break;
            }
            if ((value[2] != ':') ||
                (value[5] != ':'))
            {
                break;
            }

            uint16_t hour, minute, second;
            const std::string& hour_str = value.substr(0, 2);
            const std::string& minute_str = value.substr(3, 2);
            const std::string& second_str = value.substr(6, 2);
            if (!CStringUtils::string2int(hour_str.c_str(), hour, 0, true))
            {
                break;
            }
            if (!CStringUtils::string2int(minute_str.c_str(), minute, 0, true))
            {
                break;
            }
            if (!CStringUtils::string2int(second_str.c_str(), second, 0, true))
            {
                break;
            }
            if (hour > 23)
            {
                break;
            }
            if (minute > 59)
            {
                break;
            }
            if (second > 59)
            {
                break;
            }

            return true;
        } while(false);

        if (name().size() < 2)
            *errmsg = CStringUtils::format_string("parameter[-%s] with invalid time format (hh:mm:ss): %s", name().c_str(), value.c_str());
        else
            *errmsg = CStringUtils::format_string("parameter[--%s] with invalid time format (hh:mm:ss): %s", name().c_str(), value.c_str());
        return false;
    }

    virtual bool check_value(std::string* errmsg)
    {
        return check_value(_value, errmsg);
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        if (!check_value(new_value, errmsg))
        {
            return false;
        }
        else
        {
            _value = new_value;
            return true;
        }
    }
};

// 要求格式为：YYYY-MM-DD hh:mm:ss
class CDatetimeStringArgument: public CStringArgument
{
public:
    CDatetimeStringArgument(const std::string& name, const std::string& default_value, const std::string& help_string, bool enable_empty)
        : CStringArgument(name, default_value, help_string, enable_empty)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

private:
    bool check_value(const std::string& value, std::string* errmsg)
    {
        set_checked();

        if (value.empty())
        {
            return handle_empty_value(errmsg);
        }

        do
        {
            if (value.size() != sizeof("YYYY-MM-DD hh:mm:ss")-1)
            {
                break;
            }
            if ((value[4] != '-') ||
                (value[7] != '-') ||
                (value[10] != ' ') ||
                (value[13] != ':') ||
                (value[16] != ':'))
            {
                break;
            }

            uint16_t year, month, day, hour, minute, second;
            const std::string& year_str = value.substr(0, 4);
            const std::string& month_str = value.substr(5, 2);
            const std::string& day_str = value.substr(8, 2);
            const std::string& hour_str = value.substr(11, 2);
            const std::string& minute_str = value.substr(14, 2);
            const std::string& second_str = value.substr(17, 2);
            if (!CStringUtils::string2int(year_str.c_str(), year, 0, false) ||
                !CStringUtils::string2int(month_str.c_str(), month, 0, true) ||
                !CStringUtils::string2int(day_str.c_str(), day, 0, true) ||
                !CStringUtils::string2int(hour_str.c_str(), hour, 0, true) ||
                !CStringUtils::string2int(minute_str.c_str(), minute, 0, true) ||
                !CStringUtils::string2int(second_str.c_str(), second, 0, true))
            {
                break;
            }
            if (year < 1970)
            {
                break;
            }
            if (month > 12)
            {
                break;
            }
            if (day > 31)
            {
                break;
            }
            if (2==month && day>29)
            {
                break;
            }
            if (hour > 23)
            {
                break;
            }
            if (minute > 59)
            {
                break;
            }
            if (second > 59)
            {
                break;
            }

            return true;
        }
        while (false);

        if (name().size() < 2)
            *errmsg = CStringUtils::format_string("parameter[-%s] with invalid datetime format (YYYY-MM-DD hh:mm:ss): %s", name().c_str(), value.c_str());
        else
            *errmsg = CStringUtils::format_string("parameter[--%s] with invalid datetime format (YYYY-MM-DD hh:mm:ss): %s", name().c_str(), value.c_str());
        return false;
    }

    virtual bool check_value(std::string* errmsg)
    {
        return check_value(_value, errmsg);
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        if (!check_value(new_value, errmsg))
        {
            return false;
        }
        else
        {
            _value = new_value;
            return true;
        }
    }
};

// 值取值只能为false或true
class CBoolStringArgument: public CStringArgument
{
public:
    CBoolStringArgument(const std::string& name, const std::string& default_value, const std::string& help_string)
        : CStringArgument(name, default_value, help_string, false)
    {
        _bool_value = (default_value == "true");
        CArgumentContainer::get_singleton()->add_argument(this);
    }

    virtual bool is_true() const
    {
        return _bool_value;
    }

    virtual bool is_false() const
    {
        return _bool_value;
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        if (new_value.empty())
        {
            return handle_empty_value(errmsg);
        }
        if ((new_value != "false") && (new_value != "true"))
        {
            if (name().size() < 2)
                *errmsg = CStringUtils::format_string("parameter[-%s] with invalid value (valid: true or false): %s", name().c_str(), new_value.c_str());
            else
                *errmsg = CStringUtils::format_string("parameter[--%s] with invalid value (valid: true or false): %s", name().c_str(), new_value.c_str());
            return false;
        }

        _value = new_value;
        if (new_value == "true")
            _bool_value = true;
        else
            _bool_value = false;
        return true;
    }

private:
    bool _bool_value;
};

template <typename IntType>
class CIntArgument: public CArgumentBase
{
public:
    CIntArgument(const std::string& name, IntType default_value, IntType min_value, IntType max_value, const std::string& help_string)
        : CArgumentBase(name, help_string),
          _default_value(default_value), _min_value(min_value), _max_value(max_value), _value(default_value)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

    IntType default_value() const
    {
        return _default_value;
    }

    IntType min_value() const
    {
        return _min_value;
    }

    IntType max_value() const
    {
        return _max_value;
    }

    IntType value() const
    {
        return _value;
    }

    const std::string& str_value() const
    {
        return _str_value;
    }

public:
    virtual bool is_true() const
    {
        return _value != 0;
    }

    virtual bool is_false() const
    {
        return 0 == _value;
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        IntType value = 0;
        uint8_t converted_length = 0; // 阻止调用模板类型的string2int，在一些环境这将导致编译错误，原因是long等类型并不没有对应的带长度的类型
        bool ignored_zero = false;

        _str_value = new_value;
        if (!mooon::utils::CStringUtils::string2int(new_value.c_str(), value, converted_length, ignored_zero))
        {
            *errmsg = CStringUtils::format_string("invalid value[%s] of argument[%s]", new_value.c_str(), c_name());
            return false;
        }

        if ((value < _min_value) || (value > _max_value))
        {
            // 对于int8_t或uint8_t需要强制转换一下，否则按字符显示
            if (sizeof(IntType) == sizeof(char))
                *errmsg = any2string("value[", (int)value, "] of argument[", name(), "] not between ", (int)_min_value, " and ", (int)_max_value);
            else
                *errmsg = any2string("value[", value, "] of argument[", name(), "] not between ", _min_value, " and ", _max_value);
            return false;
        }
        else
        {
            _value = value;
            return true;
        }
    }

    virtual std::string usage_string() const
    {
        std::string prefix;
        if (name().length() < 2)
            prefix = "-";
        else
            prefix = "--";

        if (sizeof(IntType) == sizeof(char))
        {
            return mooon::utils::any2string(
                prefix, name(), "[", (int)_default_value, "/", (int)_min_value, ",", (int)_max_value, "]: ", help_string());
        }
        else
        {
            return mooon::utils::any2string(
                prefix, name(), "[", _default_value, "/", _min_value, ",", _max_value, "]: ", help_string());
        }
    }

private:
    std::string _str_value;
    IntType _default_value;
    IntType _min_value;
    IntType _max_value;
    IntType _value;
};

class CDoubleArgument: public CArgumentBase
{
public:
    CDoubleArgument(const std::string& name, double default_value, double min_value, double max_value, const std::string& help_string)
        : CArgumentBase(name, help_string),
          _default_value(default_value), _min_value(min_value), _max_value(max_value), _value(default_value)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

    double default_value() const
    {
        return _default_value;
    }

    double min_value() const
    {
        return _min_value;
    }

    double max_value() const
    {
        return _max_value;
    }

    double value() const
    {
        return _value;
    }

    const std::string& str_value() const
    {
        return _str_value;
    }

public:
    virtual bool is_true() const
    {
        return _value != 0.0;
    }

    virtual bool is_false() const
    {
        return 0.0 == _value;
    }

    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        double value = 0.0;

        _str_value = new_value;
        if (!mooon::utils::CStringUtils::string2double(new_value.c_str(), value))
        {
            *errmsg = CStringUtils::format_string("invalid value[%s] of argument[%s]", new_value.c_str(), c_name());
            return false;
        }

        if ((value < _min_value) || (value > _max_value))
        {
            *errmsg = any2string("value[", value, "] of argument[", name(), "] not between ", _min_value, " and ", _max_value);
            return false;
        }
        else
        {
            _value = value;
            return true;
        }
    }

    virtual std::string usage_string() const
    {
        std::string prefix;
        if (name().length() < 2)
            prefix = "-";
        else
            prefix = "--";

         return mooon::utils::any2string(
                prefix, name(), "[", _default_value, "/", _min_value, ",", _max_value, "]: ", help_string());
    }

private:
    std::string _str_value;
    double _default_value;
    double _min_value;
    double _max_value;
    double _value;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_ARGS_PARSER_H
