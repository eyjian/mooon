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
#include "mooon/utils/md5_helper.h"
#include "md5.h"
#include "mooon/utils/scoped_ptr.h"
UTILS_NAMESPACE_BEGIN

std::string CMd5Helper::md5(const char* format, ...)
{
    va_list ap;
    size_t size = 1024;
    ScopedArray<char> buffer(new char[size]);

    int expected = 0;
    while (true)
    {
        va_start(ap, format);
        int expected = vsnprintf(buffer.get(), size, format, ap);
        va_end(ap);

        if (expected > -1 && expected < size)
            break;

        if (expected > -1)    /* glibc 2.1 */
            size = expected + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        buffer.reset(new char[size]);
    }

    struct MD5Context md5_context;
    MD5Init(&md5_context);
    MD5Update(&md5_context, (unsigned char*)buffer.get(), (unsigned int)expected);

    unsigned char digest[16];
    MD5Final(digest, &md5_context);
    return std::string((char*)digest, sizeof(digest));
}

CMd5Helper::CMd5Helper()
{
    _md5_context = new struct MD5Context;
    MD5Init((struct MD5Context*)_md5_context);
}

CMd5Helper::~CMd5Helper()
{
    delete (struct MD5Context*)_md5_context;
}

void CMd5Helper::update(const std::string& str)
{
    MD5Update((struct MD5Context*)_md5_context, (unsigned char const *)str.data(), (unsigned int)str.size());
}

std::string CMd5Helper::to_string() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return std::string((char*)digest, sizeof(digest));
}

void CMd5Helper::to_string(char str[16]) const
{
    MD5Final((unsigned char*)str, (struct MD5Context*)_md5_context);
}

void CMd5Helper::to_bytes(unsigned char str[16]) const
{
    MD5Final(str, (struct MD5Context*)_md5_context);
}

void CMd5Helper::to_int(uint64_t* low, uint64_t* high) const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    *low = *(uint64_t*)digest;
    *high =  *(uint64_t*)(digest+sizeof(uint64_t));
}

uint64_t CMd5Helper::low() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return *(uint64_t*)digest;
}

uint64_t CMd5Helper::high() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return *(uint64_t*)(digest + sizeof(uint64_t));
}

uint64_t CMd5Helper::middle() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return *(uint64_t*)(digest + sizeof(uint32_t));
}

UTILS_NAMESPACE_END
