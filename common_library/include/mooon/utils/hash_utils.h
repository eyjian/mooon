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
#ifndef MOOON_UTILS_HASH_UTILS_H
#define MOOON_UTILS_HASH_UTILS_H
#include "mooon/utils/config.h"
UTILS_NAMESPACE_BEGIN

/** 求128类型的hash函数 */
struct uint128_hasher
{
	size_t operator ()(const uint8_t* data) const
    {
		return *((size_t*)(data+4));
	}
};
	
/** 求128类型比较函数 */
struct uint128_comparer
{
	bool operator ()(const uint8_t* lhs, const uint8_t* rhs) const
    {
		return 0 == memcmp(lhs, rhs, 16); // 16个字节
	}
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_HASH_UTILS_H
