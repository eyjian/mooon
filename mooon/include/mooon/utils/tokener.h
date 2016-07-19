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
 *
 * CTokener：普通的Token解析器，每个Token之间可以单个或多个字符分隔
 * CEnhancedTokener：可用来解析URL参数形式的
 */
#ifndef MOOON_UTILS_TOKENER_H
#define MOOON_UTILS_TOKENER_H
#include "mooon/utils/string_utils.h"
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
      * @skip_sep: 是否跳过连续的sep，即如果连接相同的sep存在，则只算作一个sep
      * 返回解析出来的Token个数，但不包括容器在解析之前已存在的
      */
    template <class ContainerType>
    static int split(ContainerType* tokens, const std::string& source, const std::string& sep, bool skip_sep=false)
    {
        if (sep.empty())
        {
            tokens->push_back(source);
        }
        else if (!source.empty())
        {
            std::string str = source;
            std::string::size_type pos = str.find(sep);

            while (true)
            {
                std::string token = str.substr(0, pos);
                tokens->push_back(token);

                if (std::string::npos == pos)
                {
                    break;
                }
                if (skip_sep)
                {
                    bool end = false;
                    while (0 == strncmp(sep.c_str(), &str[pos+1], sep.size()))
                    {
                        pos += sep.size();
                        if (pos >= str.size())
                        {
                            end = true;
                            tokens->push_back(std::string(""));
                            break;
                        }
                    }

                    if (end)
                        break;
                }

                str = str.substr(pos + sep.size());
                pos = str.find(sep);
            }
        }

        return static_cast<int>(tokens->size());
    }
};

// 解析如下形式的字符串（如果name重复，则后面的值会覆盖前面相同name的值）：
// name1=value2|name2=value3|name3=value3...
class CEnhancedTokener
{
public:
    const std::map<std::string, std::string>& tokens() const
    {
        return _name_value_pair_map;
    }

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
 * CEnhancedTokener使用示例：
#include <mooon/utils/tokener.h>

int main()
{
    using namespace mooon::utils;

    CEnhancedTokener tokener;
    std::string str = "n1=v1&n2=va2&n3=v3";
    printf("%s\n", str.c_str());

    tokener.parse(str, "&", '=');
    printf("=>%s\n", tokener["n1"].c_str()); // 期待输出：=>v1
    printf("=>%s\n", tokener["n3"].c_str()); // 期待输出：=>v3
    printf("=>%s\n", tokener["n2"].c_str()); // 期待输出：=>va2

    return 0;
}
*/

// 解析如下形式的字符串（name可以相同，且不会互覆盖）：
// name1=value2|name2=value3|name3=value3...
class CEnhancedTokenerEx
{
public:
    const std::multimap<std::string, std::string>& tokens() const
    {
        return _name_value_pair_multimap;
    }

    bool exist(const std::string& name) const
    {
        return _name_value_pair_multimap.count(name) > 0;
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

            _name_value_pair_multimap.insert(std::make_pair(name, value));
        }
    }

private:
    std::multimap<std::string, std::string> _name_value_pair_multimap;
};

// 解析如下形式的字符串：
// ip1:port1#password1,ip2:port2#password2...
// 或：
// username1@ip1:port1#password1,username2@ip2:port2#password2...
class CLoginTokener
{
public:
    struct LoginInfo
    {
        std::string username;
        std::string password;
        std::string ip;
        uint16_t port;
    };

    // source 待解析的字符串
    // token_sep Token间的分隔符，示例中“username1@ip1:port1#password1”被认为是一个Token
    // ip_port_sep IP和端口间的分隔符
    // port_passwrd_sep 端口和密码间的分隔符
    // username_ip_sep 用户名和IP间的分隔符
    // 解析成功返回Token个数
    // 要求“ip:port”必须存在，否则返回-1，但user和password是可选的，
    // 如果是一个无效的端口值，也会返回-1
    static int parse(std::vector<struct LoginInfo>* login_infos, const std::string& source, const std::string& token_sep,
                     char ip_port_sep=':', char port_password_sep='#', char username_ip_sep='@')
    {
        std::vector<std::string> tokens;
        int num_tokens = CTokener::split(&tokens, source, token_sep);

        login_infos->resize(num_tokens);
        for (int i=0; i<num_tokens; ++i)
        {
            std::string port;
            std::string::size_type pos;
            std::string token = tokens[i];
            struct LoginInfo& login_info = (*login_infos)[i];

            // username
            pos = token.find(username_ip_sep); // @
            if (std::string::npos == pos)
            {
                login_info.username.clear();
            }
            else
            {
                // 存在username
                login_info.username = token.substr(0, pos);
                token = token.substr(pos + 1);
            }

            // ip
            pos = token.find(ip_port_sep); // :
            if (std::string::npos == pos)
            {
                return -1;
            }
            else
            {
                // 存在ip
                login_info.ip = token.substr(0, pos);
                token = token.substr(pos + 1);
            }

            // port
            pos = token.find(port_password_sep); // #
            if (std::string::npos == pos)
            {
                port = token;
                login_info.password.clear();
            }
            else
            {
                port = token.substr(0, pos);
                login_info.password = token.substr(pos + 1);
            }

            if (!CStringUtils::string2int(port.c_str(), login_info.port))
                return -1;
        }

        return num_tokens;
    }

    static void print(const std::vector<struct LoginInfo>& login_infos)
    {
        if (login_infos.empty())
        {
            printf("nothing\n");
        }
        else
        {
            for (std::vector<struct LoginInfo>::size_type i=0; i<login_infos.size(); ++i)
            {
                const struct LoginInfo& login_info = login_infos[i];
                printf("[%d] => %s@%s:%u#%s\n",
                    (int)i, login_info.username.c_str(), login_info.ip.c_str(), login_info.port, login_info.password.c_str());
            }
        }
    }
};

//#include <mooon/utils/tokener.h>
/*
extern "C" int main()
{
    std::string str1 = "tom@192.168.0.1:2015#password";
    std::string str2 = "192.168.0.6:2016#password";
    std::string str3 = "tom@192.168.0.5:2017";
    std::string str4 = "192.168.0.3:2018";
    std::string str5 = "192.168.0.2";
    std::string str6 = "192.168.0.6:2016#";
    std::string str7 = "192.168.0.6:2016#pwd,192.168.0.6:2016#,192.168.0.6:2016#password";

    std::vector<struct mooon::utils::CLoginTokener::LoginInfo> login_infos;

    // str1
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str1, ",");
    printf("%s", str1.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    // str2
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str2, ",");
    printf("%s", str2.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    // str3
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str3, ",");
    printf("%s", str3.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    // str4
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str4, ",");
    printf("%s", str4.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    // str5
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str5, ",");
    printf("%s", str5.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    // str6
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str6, ",");
    printf("%s", str6.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    // str7
    login_infos.clear();
    mooon::utils::CLoginTokener::parse(&login_infos, str7, ",");
    printf("%s", str7.c_str());
    mooon::utils::CLoginTokener::print(login_infos);

    return 0;
}
*/

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_TOKENER_H
