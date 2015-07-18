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

// 使用示例：
//
// #include <mooon/utils/args_parser.h>
//
// STRING_ARG_DEFINE(ip, "127.0.0.1", "listen IP address");
// INTEGER_ARG_DEFINE(uint16_t, port, 2015, 1000, 5000, "listen port");
//
// int main(int argc, char* argv[])
// {
//     std::string errmsg;
//     if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
//     {
//         fprintf(stderr, "%s\n", errmsg.c_str());
//         exit(1);
//     }
//
//     printf("ip: %s\n", mooon::argument::ip->c_value());
//     printf("port: %u\n", mooon::argument::port->value());
//     return 0;
// }

/***
 * 字符串类型参数定义
 * @param_name: 参数名，如果命令行参数中有名字和它的相同的，则将值赋给变量ArgsParser::##param_name
 * @default_value: 默认值
 */
#define STRING_ARG_DEFINE(param_name, default_value, help_string) \
    namespace mooon { namespace argument \
    { \
        mooon::utils::CStringArgument* param_name = \
            new mooon::utils::CStringArgument( \
                #param_name, default_value, help_string); \
    }}

/***
 * 整数类型参数定义
 * @int_type: 整数类型，如int或short等
 * @param_name: 参数名，如果命令行参数中有名字和它的相同的，则将值赋给变量ArgsParser::##param_name
 * @default_value: 默认值
 * @min_value: 可取的最小值
 * @max_value: 可取的最大值
 * @help_string: 帮助信息，用来显示参数作用
 * @使用方法: ArgsParser::param_name.get_value()，
 *            假设参数名为port，则为ArgsParser::port.get_value()，
 *            port的数据类型为CArgInfo<IntegerType>
 * @提示: 各种类型整数的最大最小值使用std::numeric_limits<uint16_t>.min()系列来判断
 */
#define INTEGER_ARG_DEFINE(int_type, param_name, default_value, min_value, max_value, help_string) \
    namespace mooon { namespace argument \
    { \
        mooon::utils::CIntArgument<int_type>* param_name = \
            new mooon::utils::CIntArgument<int_type>( \
                #param_name, default_value, min_value, max_value, help_string); \
    }}

// 整数类型参数声明
#define INTEGER_ARG_DECLARE(int_type, param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern mooon::utils::CIntArgument<int_type>* param_name; \
    }}

// 整数类型参数声明
#define STRING_ARG_DECLARE(param_name) \
    namespace mooon { namespace argument /** 保证不污染全局空间 */ \
    { \
        extern mooon::utils::CStringArgument* param_name; \
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
    virtual bool set_value(const std::string& new_value, std::string* errmsg) = 0;

public:
    const std::string name() const
    {
        return _name;
    }

    const std::string help_string() const
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

private:
    std::string _name;
    std::string _help_string;
};

class CArgumentContainer
{
    SINGLETON_DECLARE(CArgumentContainer)

public:
    void add_argument(CArgumentBase* argument);
    bool set_argument(const std::string& name, const std::string& value, std::string* errmsg);

private:
    std::map<std::string, CArgumentBase*> _argument_table;
};

////////////////////////////////////////////////////////////////////////////////
class CStringArgument: public CArgumentBase
{
public:
    CStringArgument(const std::string& name, const std::string& default_value, const std::string& help_string)
        : CArgumentBase(name, help_string), _default_value(default_value), _value(default_value)
    {
        CArgumentContainer::get_singleton()->add_argument(this);
    }

    const std::string default_value() const
    {
        return _default_value;
    }

    const std::string value() const
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

public:
    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        _value = new_value;
        return true;
    }

private:
    std::string _default_value;
    std::string _value;
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

public:
    virtual bool set_value(const std::string& new_value, std::string* errmsg)
    {
        IntType value = 0;

        if (!mooon::utils::CStringUtils::string2int(new_value.c_str(), value))
        {
            *errmsg = CStringUtils::format_string("invalid value[%s] of argument[%s]", new_value.c_str(), c_name());
            return false;
        }
        if ((value < _min_value) || (value > _max_value))
        {
            *errmsg = any2string("value[", value, "] of argument[", name(), "] not between ", _min_value, " and ", _max_value);
            return false;
        }

        _value = value;
        return true;
    }

private:
    IntType _default_value;
    IntType _min_value;
    IntType _max_value;
    IntType _value;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_ARGS_PARSER_H
