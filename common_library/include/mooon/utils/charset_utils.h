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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_UTILS_CHARSET_UTILS_H
#define MOOON_UTILS_CHARSET_UTILS_H
#include "mooon/utils/exception.h"
#include <errno.h>
#include <iconv.h>
#include <string.h>
UTILS_NAMESPACE_BEGIN

class CCharsetUtils
{
public:
    // from可包含非gdb、utf8等二进制字符
    // ignore_error 是否忽略不能转换的错误
    // skip_error 在ignore_error为true的前提下，是否在输出中不包含不能被转换的字节
    // 如果from中间部分包含了不能转换的部分，当ignore_error和skip_error均为true时，则结果将不包含这部分
    static void convert(const std::string& from_charset, const std::string& to_charset,
                 const std::string& from, std::string* to, bool ignore_error=true, bool skip_error=true) throw (CException);

public:
    static void gbk_to_utf8(const std::string& from, std::string* to, bool ignore_error=true, bool skip_error=true) throw (CException);
    static void utf8_to_gbk(const std::string& from, std::string* to, bool ignore_error=true, bool skip_error=true) throw (CException);

    static void gb2312_to_utf8(const std::string& from, std::string* to, bool ignore_error=true, bool skip_error=true) throw (CException);
    static void utf8_to_gb2312(const std::string& from, std::string* to, bool ignore_error=true, bool skip_error=true) throw (CException);
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_CHARSET_UTILS_H
