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
 * 功能说明: 通过解析命令行参数，用全局变量代码对应的参数值，
 * 全局变量位于ArgsParser名字空间下，全局变量的名字和参数名相同，
 * 全局变量的类型为ArgsType
 *
 * 支持三种形式的命令行参数：
 * 1. --arg_name=arg_value，要求arg_name至少两个字符，如：--port=8080
 * 2. --arg_name，不带参数值，要求arg_name至少两个字符，如：--color
 * 3. -arg_name，不带参数值，这种情况下要求arg_name为单个字符，如：-p
 *
 * 定义命令行参数是必须的，而声明根据需要可选
 *
 * 使用方法：
 * 假设需要一个端口号和IP命令参数，数据类型分别为uint16_t和std::string，则可定义为：
 *STRING_ARG_DEFINE(true, ip, 127.0.0.1, listen IP)
 *INTEGER_ARG_DEFINE(true, uint16_t, port, 8080, 1000, 6554, listen port)
 *注:字符串不需要用""括起来
 * 上面这段定义，通常和main函数放在同一个文件中。
 *
 * 如果多个文件需要使用到命令行参数，则可以使用声明的方式，如：
 * INTEGER_ARG_DECLARE(uint16_t, port)
 * STRING_ARG_DECLARE(ip)
 *
 * 在需要使用到命令行参数的地方，只需要这样：
 * ArgsParser::port->get_value()
 * ArgsParser::ip->get_value()
 *
 *其它接口：
 *ArgsParser::get_help_string() 获取命名行的帮助信息
 *ArgsParser::g_error_message 解析的错误信息
 * 示例：
 *
 * INTEGER_ARG_DEFINE(true, uint16_t, port, 8080, 1000, 6554, listen port)
 * STRING_ARG_DEFINE(true, ip, 127.0.0.1, listen IP)
 *
 * int main(int argc, char* argv[])
 * {
 *    if (!ArgsParser::parse(argc, argv))
 *    {
 *        // 解析失败，可能是某参数不符合要求，
 *        // 或者是必选参数不存在，
 *        // 或者是出现未定义的参数
 *        // 或者是出现重复的参数
 *        exit(1);
 *    }
 *
 *    // 使用
 *    ArgsParser::port->get_value(); // 这里的get_value返回的是uint16_t类型
 *    ArgsParser::ip->get_value(); // 这里的get_value返回的是std::string类型
 *
 *    return 0;
 * }
 *
 * Author: weijingqi  kekimail@gmail.com
 *
 */
#ifndef UTIL_ARGS_PARSER_H
#define UTIL_ARGS_PARSER_H
#include <map> // 用来存储命令行参数名和它对应的值信息
#include "util/string_util.h"

/***
 * 整数类型参数定义
 * @optional: 是否为可选参数，取值为true或false。如果是可选参数，则命令行参数中必须包含它
 * @integer_type: 整数类型，如int或short等
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
#define INTEGER_ARG_DEFINE(optional, integer_type, param_name, default_value, min_value, max_value, help_string) \
    namespace ArgsParser /** 保证不污染全局空间 */ \
    { \
        util::CArgInfo<integer_type> *param_name= new util::CIntArgInfo<integer_type>(optional,#param_name, default_value,min_value,max_value,#help_string);\
    }

/***
 * 字符串类型参数定义
 * @optional: 是否为可选参数
 * @param_name: 参数名，如果命令行参数中有名字和它的相同的，则将值赋给变量ArgsParser::##param_name
 * @default_value: 默认值
 */
#define STRING_ARG_DEFINE(optional, param_name, default_value, help_string)\
    namespace ArgsParser /** 保证不污染全局空间 */ \
    { \
		util::CArgInfo<std::string> *param_name= new util::CStringArgInfo<std::string>(optional,#param_name,#default_value,#help_string);\
    }

// 整数类型参数声明
#define INTEGER_ARG_DECLARE(integer_type, param_name) \
    namespace ArgsParser /** 保证不污染全局空间 */ \
    { \
        extern util::CArgInfo<integer_type> *param_name; \
    }

// 整数类型参数声明
#define STRING_ARG_DECLARE(param_name) \
    namespace ArgsParser /** 保证不污染全局空间 */ \
    { \
        extern util::CArgInfo<std::string> *param_name; \
    }

//////////////////////////////////////////////////////////////////////////

// 参数信息接口
class IArgInfo
{
public:
	virtual ~IArgInfo()
	{
	}

	/** 设置参数值 */
	virtual void set_value(const std::string& value) = 0;

	/** 得到参数名 */
	virtual const std::string& get_param_name() const = 0;

    /** 设置命令行中有该参数 */
    virtual void set() = 0;

	/** 判断命令行中是否有该参数*/
	virtual bool is_set() const = 0;

	/** 判断参数是否有值 */
	virtual bool has_value() const = 0;

	/** 判断是否为可选参数 */
	virtual bool is_optional() const = 0;

	virtual std::string to_string() = 0;

	/** 获取帮助 */
	virtual const std::string& get_help_string() const = 0;

	/**
	 * 验证value是否符合该参数
	 */
	virtual bool validate_value(const std::string& value_str) const = 0;
};

//解析器命名空间
namespace ArgsParser {

/***
  * 出错信息
  */
extern std::string g_error_message;

/***
 * 解析命令行参数，
 * 参数和main函数相同，通过在main中调用它
 * @return: 如果解析成功，则返回true，否则返回false
 */
bool parse(int argc, char* argv[]);

/***
 * 注册参数，
 * 不要直接调用它，而应当总是由宏来调用
 */
void register_arg(const std::string& param_name, IArgInfo* arg_info);

/**
 * 获取帮助信息
 */
std::string get_help_info();

} // The end of namespace ArgsParser

UTIL_NAMESPACE_BEGIN
////////////下面为参数具体实现///////
/***
 * 参数信息类
 */

template <typename DataType>
class CArgInfo: public IArgInfo
{
public:
    CArgInfo(const std::string& param_name);

	/**设置参数值*/
	void set_value(const std::string& value);

	/**得到参数名*/
	const std::string& get_param_name() const;

	/** 得到参数的值 */
	DataType get_value() const;

	/** 得到默认参数的值 */
	DataType get_default_value() const;

	/** 判断命令行中是否有该参数*/
	bool is_set() const;

	/**设置命令行中有该参数*/
	void set();

	/** 判断参数是否有值 */
	bool has_value() const;

	/** 判断是否为可选参数 */
	bool is_optional() const;

	std::string to_string();
	/**
	 * 验证value是否符合该参数
	 */
	bool validate_value(const std::string& value_str) const;

	const std::string& get_help_string() const;

protected:
    bool _is_set;
    bool _optional;
	bool _has_value;
    DataType _value;
	DataType _default_value;
    std::string _param_name;
	std::string _help_string;
};

/***
 * 参数信息类
 */
template <typename DataType>
class CStringArgInfo: public CArgInfo<DataType>
{
	typedef CArgInfo<DataType> parent_cArgInfo;

public:
	CStringArgInfo(bool optional
                 , const std::string& param_name
                 , const std::string& default_value
                 , std::string help_string);

	/** 设置参数值 */
	void set_value(const std::string& value);

	std::string to_string();

	/**
	 * 验证value是否符合该参数
	 */
	bool validate_value(const std::string& value) const;
};

/***
 * 参数信息类
 */
template <typename DataType>
class CIntArgInfo: public CArgInfo<DataType>
{
    typedef CArgInfo<DataType> parent_cArgInfo;

public:
	CIntArgInfo(bool optional
              , std::string param_name
              , DataType default_value
              , DataType min_value
              , DataType max_value
              , std::string help_string);

	/** 设置参数值 */
	void set_value(const std::string& value);

	std::string to_string();

	/**
	 * 验证value是否符合该参数
	 */
	bool validate_value(const std::string& value_str) const;

private:
	DataType _min_value;
	DataType _max_value;
};

// 类的实现
template<typename DataType>
CArgInfo<DataType>::CArgInfo(const std::string& param_name)
{
	_param_name = param_name;
	ArgsParser::register_arg(_param_name, this);
}

/** 设置参数值 */
template<typename DataType>
void CArgInfo<DataType>::set_value(const std::string& value)
{
	_has_value = true;
}

/** 得到参数名 */
template<typename DataType>
const std::string& CArgInfo<DataType>::get_param_name() const
{
	return _param_name;
}

/** 得到参数的值 */
template <typename DataType>
DataType CArgInfo<DataType>::get_value() const
{
	return _value;
}

/** 得到默认参数的值 */
template <typename DataType>
DataType CArgInfo<DataType>::get_default_value() const
{
	return _default_value;
}

/** 判断命令行中是否有该参数 */
template<typename DataType>
bool CArgInfo<DataType>::is_set() const
{
	return _is_set;
}

/** 设置命令行中有该参数 */
template<typename DataType>
void CArgInfo<DataType>::set()
{
	_is_set = true;
}

/** 判断参数是否有值 */
template<typename DataType>
bool CArgInfo<DataType>::has_value() const
{
	return _has_value;
}

/** 判断是否为可选参数 */
template<typename DataType>
bool CArgInfo<DataType>::is_optional() const
{
	return _optional;
}

template<typename DataType>
std::string CArgInfo<DataType>::to_string()
{
	return "";
}

/**
 * 验证value是否符合该参数
 */
template<typename DataType>
bool CArgInfo<DataType>::validate_value(const std::string& value_str) const
{
	return false;
}

template<typename DataType>
const std::string& CArgInfo<DataType>::get_help_string() const
{
	return _help_string;
}

template<typename DataType>
CStringArgInfo<DataType>::CStringArgInfo(bool optional
		                               , const std::string& param_name
		                               , const std::string& default_value
		                               , std::string help_string)
    :CArgInfo<DataType>(param_name)
{
	parent_cArgInfo::_optional = optional;
	parent_cArgInfo::_has_value = false;
	parent_cArgInfo::_is_set = false;
	parent_cArgInfo::_default_value = default_value;
	parent_cArgInfo::_help_string = help_string;
}

/** 设置参数值 */
template<typename DataType>
void CStringArgInfo<DataType>::set_value(const std::string& value)
{
	parent_cArgInfo::_has_value = true;
	parent_cArgInfo::_value = value;
}

template<typename DataType>
std::string CStringArgInfo<DataType>::to_string()
{
	return parent_cArgInfo::_value;
}

/**
 * 验证value是否符合该参数
 */
template<typename DataType>
bool CStringArgInfo<DataType>::validate_value(const std::string& value) const
{
	return (!value.empty());
}

template<typename DataType>
CIntArgInfo<DataType>::CIntArgInfo(bool optional
		                         , std::string param_name
		                         , DataType default_value
		                         , DataType min_value
		                         , DataType max_value
		                         , std::string help_string)
    :CArgInfo<DataType>(param_name)
{
	parent_cArgInfo::_is_set = false;
	parent_cArgInfo::_has_value = false;
	parent_cArgInfo::_optional = optional;

	parent_cArgInfo::_optional = optional;
	parent_cArgInfo::_help_string = help_string;
    parent_cArgInfo::_value = 0;
	parent_cArgInfo::_default_value = default_value;

	_max_value = max_value;
	_min_value = min_value;
}

/** 设置参数值 */
template<typename DataType>
void CIntArgInfo<DataType>::set_value(const std::string& value)
{
	parent_cArgInfo::_has_value = true;
	CStringUtil::string2int(value.data(), parent_cArgInfo::_value,value.length(), true);
}

template<typename DataType>
std::string CIntArgInfo<DataType>::to_string()
{
	return CStringUtil::int_tostring(parent_cArgInfo::_value);
}

/**
 * 验证value是否符合该参数
 */
template<typename DataType>
bool CIntArgInfo<DataType>::validate_value(const std::string& value_str) const
{
	DataType value;
	if (CStringUtil::string2int(value_str.data(), value, value_str.length(), true))
	{
		if ((value < _min_value) || (value > _max_value))
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

UTIL_NAMESPACE_END
#endif // UTIL_ARGS_PARSER_H
