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
 * Author: eyjian@qq.com, eyjian@gmail.com
 */
#include "utils/args_parser.h"
namespace ArgsParser {

std::string g_error_message;
static std::map<std::string, IArgInfo*>& ArgsInfoMap()
{
	static std::map<std::string, IArgInfo*> s_ArgsInfoMap;
	return s_ArgsInfoMap;
}

bool parse(int argc, char* argv[])
{
    char* param;
    bool with_value;
    std::string name;
	std::string param_str;
	std::string value_str;

	for (int i=1; i<argc; ++i)
	{
		param = argv[i];
		with_value = false;
        value_str.clear();
		name.clear();

		//--开始
		if (('-' == param[0]) && ('-' == param[1]))
		{
			param_str = param;
			param_str = param_str.substr(2);
			int index = param_str.find('=');

			if (index)
			{
				name = param_str.substr(0, index);
				value_str = param_str.substr(index + 1);
				with_value = true;
			}
            else
			{
				name = param_str;
			}

			// 名字长度须大于2
			if (name.length() < 2)
			{
				g_error_message += "Error:" + (std::string)param + " the min length of arg name should be 2";
				return false;
			}
		}
        else if (param[0] == '-') // -开始
		{
			param_str = param;
			param_str = param_str.substr(1);
			name = param_str;

			if (name.length() != 1)
			{
				g_error_message += "Error:" + (std::string)param + " the length of arg name should be 1";
				return false;
			}
		}

		if (name.empty())
		{
			g_error_message += "Error:" + (std::string)param + " arg name can not be null";
			return false;
		}

		// 按具体参数的规则判断
		if (ArgsInfoMap().find(name) == ArgsInfoMap().end())
		{
			g_error_message += "Error:" + (std::string)param + " the command rule not contains: " + name;
			return false;
		}

		// 判断value
		if (with_value)
		{
			if (!ArgsInfoMap().find(name)->second->validate_value(value_str))
			{
				g_error_message += std::string("Error:") + std::string(param) + " the value of " + name + " is not valid";
				return false;
			}

			ArgsInfoMap().find(name)->second->set_value(value_str);
		}

		ArgsInfoMap().find(name)->second->set();
	}

	// 检测必填参数是否都填上
	std::map<std::string, IArgInfo*>::iterator iter = ArgsInfoMap().begin();
    for (; iter!=ArgsInfoMap().end(); ++iter)
	{
		if (!iter->second->is_optional()
         && !iter->second->is_set())
		{
			g_error_message += std::string("Error: param ") + iter->second->get_param_name() + std::string(" not set");
			return false;
		}
	}

    return true;
}

/***
 * 注册参数，
 * 不要直接调用它，而应当总是由宏来调用
 */
void register_arg(const std::string& param_name, IArgInfo* arg_info)
{
	ArgsInfoMap().insert(std::make_pair(param_name, arg_info));
}

/**
 * 获取帮助信息
 */
std::string get_help_info()
{
    std::string name;
	std::string info = "Options:\r\n";
	std::string optional;

	IArgInfo* argInfo;
    std::map<std::string, IArgInfo*>::iterator iter = ArgsInfoMap().begin();

	for (; iter!=ArgsInfoMap().end(); ++iter)
	{
		argInfo = iter->second;
		name = argInfo->get_param_name();
		if(argInfo->is_optional())
		{
			optional = "true";
		}
		else
		{
			optional = "false";
		}
		if (name.length() == 1)
		{
			info += "\t-" + name + "\t" + "optional:" + optional + "\t" + argInfo->get_help_string()
					+ "\r\n";
		}
        else
		{
			info += "\t--" + name + "\t" + "optional:" + optional + "\t" + argInfo->get_help_string()
					+ "\r\n";
		}
	}

	return info;
}

} // namespace ArgsParser
