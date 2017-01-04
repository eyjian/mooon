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

// 使用示例：
// uint64_t url2int(const std::string& url)
// {
//     CMd5Helper md5_helper;
//     md5_helper.update(url);
//     return md5_helper.low_8bytes();
// }
class CMd5Helper
{
public:
    static std::string md5(const char* format, ...) __attribute__((format(printf, 1, 2)));
    static std::string lowercase_md5(const char* format, ...) __attribute__((format(printf, 1, 2)));
    static std::string uppercase_md5(const char* format, ...) __attribute__((format(printf, 1, 2)));

public:
    struct Value
	{
    	uint64_t low_8bytes;
    	uint64_t high_8bytes;

    	Value()
    		: low_8bytes(0), high_8bytes(0)
    	{
    	}

    	Value(const Value& other)
    	{
    		this->low_8bytes = other.low_8bytes;
    		this->high_8bytes = other.high_8bytes;
    	}

    	Value& operator =(const Value& other)
    	{
    		this->low_8bytes = other.low_8bytes;
    		this->high_8bytes = other.high_8bytes;

    		return *this;
    	}

    	bool operator ==(const Value& other) const
		{
    		return (this->low_8bytes == other.low_8bytes) && (this->high_8bytes == other.high_8bytes);
		}

    	bool operator !=(const Value& other) const
		{
			return (this->low_8bytes != other.low_8bytes) || (this->high_8bytes != other.high_8bytes);
		}

    	bool operator <(const Value& other) const
    	{
    	    if (this->low_8bytes < other.low_8bytes)
    	        return true;
    	    if (this->low_8bytes == other.low_8bytes)
    	        return this->high_8bytes < other.high_8bytes;
    	    return false;
    	}

    	bool operator >(const Value& other) const
    	{
    	    return !(*this < other);
    	}

    	bool operator <=(const Value& other) const
        {
    	    if (this->low_8bytes < other.low_8bytes)
    	        return true;
    	    if (this->low_8bytes == other.low_8bytes)
    	        return this->high_8bytes <= other.high_8bytes;
    	    return false;
        }

    	bool operator >=(const Value& other) const
        {
    	    return !(*this <= other);
        }
	};

    // 求哈希值函数
    typedef struct
    {
        uint64_t operator ()(const struct Value& value) const
        {
            return value.low_8bytes;
        }
    }ValueHasher;

    // 哈希比较函数
    typedef struct
    {
        bool operator()(const struct Value& lhs, const struct Value& rhs) const
        {
            return lhs == rhs;
        }
    }ValueComparer;

public:
    CMd5Helper();
    ~CMd5Helper();

    void update(const char* str, size_t size);
    void update(const std::string& str);

public:
    void to_string(char str[33], bool uppercase=true) const;
    std::string to_string(bool uppercase=true) const;
    void to_bytes(unsigned char str[16]) const;
    void to_int(uint64_t* low_8bytes, uint64_t* high_8bytes) const;

    struct Value value() const;

    // 取低8字节
    uint64_t low_8bytes() const;

    // 取高8字节
    uint64_t high_8bytes() const;

    // 取中间8字节
    uint64_t middle_8bytes() const;

private:
    void* _md5_context;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_MD5_HELPER_H
