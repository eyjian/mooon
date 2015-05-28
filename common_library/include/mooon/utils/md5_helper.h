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
#ifndef MOOON_UTILS_MD5_HELPER_H
#define MOOON_UTILS_MD5_HELPER_H
#include "mooon/utils/string_utils.h"
#include <stdarg.h>
#include <stdint.h>
UTILS_NAMESPACE_BEGIN

class CMd5Helper
{
public:
    static std::string md5(const char* format, ...) __attribute__((format(printf, 1, 2)));

public:
    CMd5Helper();
    ~CMd5Helper();
    void update(const std::string& str);

public:
    std::string to_string() const;
    void to_string(char str[16]) const;
    void to_bytes(unsigned char str[16]) const;
    void to_int(uint64_t* low, uint64_t* high) const;

    // 取低8字节
    uint64_t low() const;

    // 取高8字节
    uint64_t high() const;

    // 取中间8字节
    uint64_t middle() const;

private:
    void* _md5_context;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_MD5_HELPER_H
