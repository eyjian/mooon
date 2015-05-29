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
	int expected = 0;
    size_t size = 1024;
    ScopedArray<char> buffer(new char[size]);
    va_list ap;

    while (true)
    {
        va_start(ap, format);
        expected = vsnprintf(buffer.get(), size, format, ap);
        va_end(ap);

        if (expected > -1 && expected < (int)size)
            break;

        if (expected > -1)    /* glibc 2.1 */
            size = expected + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        buffer.reset(new char[size]);
    }

    CMd5Helper md5_helper;
    md5_helper.update(buffer.get(), expected);
    return md5_helper.to_string();
}

std::string CMd5Helper::lowercase_md5(const char* format, ...)
{
	int expected = 0;
    size_t size = 1024;
    ScopedArray<char> buffer(new char[size]);
    va_list ap;

    while (true)
    {
        va_start(ap, format);
        expected = vsnprintf(buffer.get(), size, format, ap);
        va_end(ap);

        if (expected > -1 && expected < (int)size)
            break;

        if (expected > -1)    /* glibc 2.1 */
            size = expected + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        buffer.reset(new char[size]);
    }

    CMd5Helper md5_helper;
    md5_helper.update(buffer.get(), expected);
    return md5_helper.to_string(false);
}

std::string CMd5Helper::uppercase_md5(const char* format, ...)
{
	int expected = 0;
    size_t size = 1024;
    ScopedArray<char> buffer(new char[size]);
    va_list ap;

    while (true)
    {
        va_start(ap, format);
        expected = vsnprintf(buffer.get(), size, format, ap);
        va_end(ap);

        if (expected > -1 && expected < (int)size)
            break;

        if (expected > -1)    /* glibc 2.1 */
            size = expected + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        buffer.reset(new char[size]);
    }

    CMd5Helper md5_helper;
    md5_helper.update(buffer.get(), expected);
    return md5_helper.to_string(true);
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

void CMd5Helper::update(const char* str, size_t size)
{
    MD5Update((struct MD5Context*)_md5_context, (unsigned char const *)str, size);
}

void CMd5Helper::update(const std::string& str)
{
	update(str.c_str(), str.size());
}

void CMd5Helper::to_string(char str[33], bool uppercase) const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    // ע��snprintf()�Ĳ���������
    for (int i=0; i<16; ++i)
    {
    	if (uppercase)
    		(void)snprintf(str + i*2, 3, "%02X", digest[i]);
    	else
    		(void)snprintf(str + i*2, 3, "%02x", digest[i]);
    }

    str[32] = '\0';
}

std::string CMd5Helper::to_string(bool uppercase) const
{
	char str[33];
	to_string(str, uppercase);
    return str;
}

void CMd5Helper::to_bytes(unsigned char str[16]) const
{
    MD5Final(str, (struct MD5Context*)_md5_context);
}

void CMd5Helper::to_int(uint64_t* low_8bytes, uint64_t* high_8bytes) const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    *low_8bytes = *(uint64_t*)digest;
    *high_8bytes =  *(uint64_t*)(digest+sizeof(uint64_t));
}

struct CMd5Helper::Value CMd5Helper::value() const
{
	Value value;
	to_int(&value.low_8bytes, &value.high_8bytes);
	return value;
}

uint64_t CMd5Helper::low_8bytes() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return *(uint64_t*)digest;
}

uint64_t CMd5Helper::high_8bytes() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return *(uint64_t*)(digest + sizeof(uint64_t));
}

uint64_t CMd5Helper::middle_8bytes() const
{
    unsigned char digest[16];
    MD5Final(digest, (struct MD5Context*)_md5_context);

    return *(uint64_t*)(digest + sizeof(uint32_t));
}

UTILS_NAMESPACE_END
