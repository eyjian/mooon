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
#ifndef MOOON_UTILS_TOKENER_H
#define MOOON_UTILS_TOKENER_H
#include "mooon/utils/config.h"
#include <list>
#include <map>
#include <string>
#include <vector>
UTILS_NAMESPACE_BEGIN

class CTokener
{
public:
    /***
      * 以指定的字符串作为分隔符，将整个字符中各Token解析到一个容器中
      * @tokens: 存储Token的容器，可为list或vector等，只要它支持push_back()
      * @source: 被解析的字符串
      * @sep: Token分隔符
      * 返回解析出来的Token个数，但不包括容器在解析之前已存在的
      */
    template <class ContainerType>
    static int split(ContainerType* tokens, const std::string& source, const std::string& sep)
    {
        int num_tokens = 0;

        if (!source.empty())
        {
            std::string str = source;
            std::string::size_type pos = str.find(sep);

            while (true)
            {
                std::string token = str.substr(0, pos);
                tokens->push_back(token);
                ++num_tokens;

                if (std::string::npos == pos)
                {
                    break;
                }

                str = str.substr(pos + sep.size());
                pos = str.find(sep);
            }
        }

        return static_cast<int>(tokens->size());
    }
};

// 解析如下形式的字符串：
// name1=value2|name2=value3|name3=value3...
class CEnhancedCTokener
{
public:
    std::string operator [](const std::string& name) const
    {
        return get(name);
    }
    
    bool exist(const std::string& name) const
    {
        return _name_value_pair_map.count(name) > 0;
    }
    
    bool get(const std::string& name, std::string* value) const
    {
        std::map<std::string, std::string>::const_iterator iter = _name_value_pair_map.find(name);
        if (iter != _name_value_pair_map.end())
        {
            *value = iter->second;
            return true;
        }
        
        return false;
    }
    
    std::string get(const std::string& name) const
    {
        std::string value;
        (void)get(name, &value);
        return value;
    }
    
    void parse(const std::string& source, const std::string& token_sep, char name_value_sep='=')
    {
        std::vector<std::string> tokens;        
        int num_tokens = CTokener::split(&tokens, source, token_sep);
        
        for (int i=0; i<num_tokens; ++i)
        {
            const std::string& token = tokens[i];
            std::string::size_type pos = token.find(name_value_sep);
            
            std::string name;
            std::string value;
            if (pos == std::string::npos)
            {
                name = token;
            }
            else
            {
                name = token.substr(0, pos);
                value = token.substr(pos + 1);
            }

            std::pair<std::map<std::string, std::string>::iterator, bool> ret =
                _name_value_pair_map.insert(std::make_pair(name, value));
            if (!ret.second)
            {
                // 用后面的覆盖前面的
                ret.first->second = value;
            }
        }
    }
    
private:
    std::map<std::string, std::string> _name_value_pair_map;
};

/*
 * 使用示例：
#include <mooon/utils/tokener.h>

int main()
{
    using namespace mooon::utils;

    CEnhancedCTokener tokener;
    std::string str = "n1=v1&n2=va2&n3=v3";
    printf("%s\n", str.c_str());

    tokener.parse(str, "&", '=');
    printf("=>%s\n", tokener["n1"].c_str()); // 期待输出：=>v1
    printf("=>%s\n", tokener["n3"].c_str()); // 期待输出：=>v3
    printf("=>%s\n", tokener["n2"].c_str()); // 期待输出：=>va2

    return 0;
}
*/

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_TOKENER_H
