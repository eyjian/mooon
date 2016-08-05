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
#ifndef MOOON_UTILS_REGEX_HELPER_H
#define MOOON_UTILS_REGEX_HELPER_H
#include <mooon/utils/config.h>
#include <mooon/utils/exception.h>
#include <regex.h>
#include <string.h>
#include <sys/types.h>
UTILS_NAMESPACE_BEGIN

class CRegexHelper
{
public:
    // cflags可选值：REG_EXTENDED, REG_ICASE, REG_NOSUB, REG_NEWLINE，可多个按位组合
    CRegexHelper(const char* pattern, int cflags=REG_EXTENDED) throw (CException);
    ~CRegexHelper();
    std::string get_error_message(int errcode);

    // eflags可选值：REG_NOTBOL, REG_NOTEOL，两种可按位组合
    bool match(const char* str, int eflags=REG_NOTBOL);

private:
    regex_t _regex;
};

CRegexHelper::CRegexHelper(const char* pattern, int cflags) throw (CException)
{
    int errcode = regcomp(&_regex, pattern, cflags);
	if (errcode != 0)
		THROW_EXCEPTION(get_error_message(errcode), errcode);
}

CRegexHelper::~CRegexHelper()
{
    regfree(&_regex);
}

bool CRegexHelper::match(const char* str, int eflags)
{
    regmatch_t match[1];
    int errcode = regexec(&_regex, str, 1, match, eflags);
    return (0 == errcode);
}

std::string CRegexHelper::get_error_message(int errcode)
{
    char errbuf[1024];
    regerror(errcode, &_regex, errbuf, sizeof(errbuf)-1);
    return errbuf;
}

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_REGEX_HELPER_H
