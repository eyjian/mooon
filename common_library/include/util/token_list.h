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
#ifndef MOOON_UTILS_TOKEN_LIST_H
#define MOOON_UTILS_TOKEN_LIST_H
#include <list>
#include "util/config.h"
UTILS_NAMESPACE_BEGIN

/***
  * 将字符串按指定的分隔字符串解析到一个链表中
  */
class CTokenList
{
public:
    /** 存储Token的链表 */
    typedef std::list<std::string> TTokenList;

    /***
      * 以指定的字符串作为分隔符，将整个字符中各Token解析到一个Token链表中
      * @token_list: 存储Token的链表
      * @source: 被解析的字符串
      * @sep: Token分隔符
      */
    static void parse(TTokenList& token_list, const std::string& source, const std::string& sep);
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_TOKEN_LIST_H
