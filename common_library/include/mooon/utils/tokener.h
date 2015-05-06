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

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_TOKENER_H
