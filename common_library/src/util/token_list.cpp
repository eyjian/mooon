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
#include "util/token_list.h"
UTIL_NAMESPACE_BEGIN

void CTokenList::parse(TTokenList& token_list, const std::string& source, const std::string& sep)
{    
    if (!source.empty())
    {        
        std::string str = source; 
        std::string::size_type pos = str.find(sep);

        while (true)
        {
            std::string token = str.substr(0, pos);
            token_list->push_back(token);

            if (std::string::npos == pos)
            {   
                break;
            }

            str = str.substr(pos + sep.size());
            pos = str.find(sep);
        }
    }
}

/*
void print(const std::string& source, const std::string& sep)
{
    std::vector<std::string> tokens;
    int num_tokens = split(source, sep, &tokens);

    printf("source[%s]:\n", source.c_str());
    for (int i=0; i<num_tokens; ++i)
    {
        printf("token: \"%s\"\n", tokens[i].c_str());
    }

    printf("\n");
}

int main()
{
    std::string str1 = "abc##123##x#z##456";
    print(str1, "##");

    std::string str2 = "##abc##123##x#z##456";
    print(str2, "##");

    std::string str3 = "##abc##123##x#z##456##";
    print(str3, "##");

    std::string str4 = "###abc###123##x#z##456##";
    print(str4, "##");

    std::string str5 = "###abc###123##x#z###456##";
    print(str5, "###");

    std::string str6 = "##abc####123##x#z##456";
    print(str6, "##");

    std::string str7 = "##";
    print(str7, "##");

    std::string str8 = "";
    print(str8, "##");

    std::string str9 = "#abc#123#x#z#456#";
    print(str9, "#");

    std::string str0 = "##abc######123##x#z##456####";
    print(str0, "##");

    return 0;
}
*/

UTIL_NAMESPACE_END
