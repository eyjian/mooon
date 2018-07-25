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
#include "utils/args_parser.h"
#include <sstream>
UTILS_NAMESPACE_BEGIN

std::string g_help_string; // --help的说明
std::string g_version_string; // --version的说明

// 跳过前缀
static std::string remove_prefix_of_argument_name(const std::string& argument_name)
{
    std::string::size_type pos = 0;
    std::string new_argument_name;

    for (pos=0; pos<argument_name.size(); ++pos)
    {
        if (argument_name[pos] != '-')
            break;
    }

    new_argument_name = argument_name.substr(pos);
    return new_argument_name;
}

// 解析命令行参数
bool parse_arguments(int argc, char* argv[], std::string* errmsg)
{
    MOOON_ASSERT(errmsg != NULL);

    g_help_string = mooon::utils::CArgumentContainer::get_singleton()->usage_string();
    for (int i=1; i<argc; ++i)
    {
        std::string argument = argv[i];
        std::string argument_name;
        std::string argument_value;

        // 只找第一个等号，这样参数值中有等号时可正常运行
        std::string::size_type equal_sign_pos = argument.find("=");
        if (std::string::npos == equal_sign_pos)
        {
            argument_name = argument;
        }
        else
        {
            argument_name = argument.substr(0, equal_sign_pos);
            argument_value = argument.substr(equal_sign_pos + 1);
        }

        // 对help和version做内置处理
        argument_name = remove_prefix_of_argument_name(argument_name);
        if ("version" == argument_name)
        {
            *errmsg = g_version_string;
            return false;
        }
        if ("help" == argument_name)
        {
            errmsg->clear();
            return false;
        }
        if (!mooon::utils::CArgumentContainer::get_singleton()->set_argument(
                argument_name, argument_value, errmsg))
        {
            return false;
        }
    }

    return mooon::utils::CArgumentContainer::get_singleton()->check_parameters(errmsg);
}

////////////////////////////////////////////////////////////////////////////////
CArgumentBase::CArgumentBase(const std::string& name, const std::string& help_string)
    : _checked(false), _name(name), _help_string(help_string)
{
}

////////////////////////////////////////////////////////////////////////////////
SINGLETON_IMPLEMENT(CArgumentContainer);

CArgumentContainer::~CArgumentContainer()
{
    for (std::map<std::string, CArgumentBase*>::iterator iter=_argument_table.begin(); iter!=_argument_table.end(); ++iter)
    {
        CArgumentBase* arg = iter->second;
        delete arg;
    }
}

void  CArgumentContainer::add_argument(CArgumentBase* argument)
{
    std::pair<ArgumentTable::iterator, bool> ret = _argument_table.insert(std::make_pair(argument->name(), argument));
    if (ret.second)
    {
        _argument_list.push_back(argument);
    }
}

bool CArgumentContainer::set_argument(const std::string& name, const std::string& value, std::string* errmsg)
{
    CArgumentBase* argument = NULL;
    ArgumentTable::iterator iter = _argument_table.find(name);

    if (iter == _argument_table.end())
    {
        *errmsg = CStringUtils::format_string("undefined argument: %s", name.c_str());
        return false;
    }
    else
    {
        argument = iter->second;
        argument->set_checked();
        return argument->set_value(value, errmsg);
    }
}

std::string CArgumentContainer::usage_string() const
{
    std::stringstream usage_stream;

    usage_stream << "usage:" << std::endl;
    for (ArgumentList::size_type i=0; i<_argument_list.size(); ++i)
    {
        CArgumentBase* argument = _argument_list[i];
        usage_stream << argument->usage_string() << std::endl;
    }

    return usage_stream.str();
}

bool CArgumentContainer::check_parameters(std::string* errmsg) const
{
    for (ArgumentList::size_type i=0; i<_argument_list.size(); ++i)
    {
        CArgumentBase* argument = _argument_list[i];

        if (!argument->checked())
        {
            if (!argument->check_value(errmsg))
            {
                return false;
            }
        }
    }

    return true;
}

UTILS_NAMESPACE_END
