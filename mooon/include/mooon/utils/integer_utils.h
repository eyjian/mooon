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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_UTILS_INTEGER_UTILS_H
#define MOOON_UTILS_INTEGER_UTILS_H
#include "mooon/utils/config.h"
#include <math.h>
UTILS_NAMESPACE_BEGIN

/***
  * 整数数字操作工具类
  */
class CIntegerUtils
{
public:	
    template <typename DataType>
    static bool is_prime_number(DataType x)
    {
        if (x < 2)
            return false;

        DataType k = (DataType)sqrt(x);
        for (DataType m=2; m<=k; ++m)
            if (0 == x % m)
                return false;

        return true;
    }

    /** 返回不超过指定宽度的整数，如果超过则溢出从0开始  */
    template <typename IntType>
    static IntType dec_with_width(IntType m, int width)
    {
        // __UINT64_C(999999999)
        // 使用宏__UINT64_C，编译时需要定义__STDC_LIMIT_MACROS
        MOOON_ASSERT(width > 0 && width < 10 && m >= 0);
        static uint64_t dec_width_table[] = { (__UINT64_C(0)), (__UINT64_C(9)), (__UINT64_C(99)), (__UINT64_C(999)), (__UINT64_C(9999)), (__UINT64_C(99999)), (__UINT64_C(999999)), (__UINT64_C(9999999)), (__UINT64_C(99999999)), (__UINT64_C(999999999)) };
        return static_cast<IntType>(m % dec_width_table[width]);
    }

    template <typename IntType>
    static IntType hex_with_width(IntType m, int width)
    {
        // 0x999999999 == 41231686041
        MOOON_ASSERT(width > 0 && width < 10 && m >= 0);
        static uint64_t hex_width_table[] = { (__UINT64_C(0x0)), (__UINT64_C(0x9)), (__UINT64_C(0x99)), (__UINT64_C(0x999)), (__UINT64_C(0x9999)), (__UINT64_C(0x99999)), (__UINT64_C(0x999999)), (__UINT64_C(0x9999999)), (__UINT64_C(0x99999999)), (__UINT64_C(0x999999999)) };
        return static_cast<IntType>(m % hex_width_table[width]);
    }

    /** 判断一个数字是否可为int16_t数字 */
    static bool is_int16(int32_t num);

    /** 判断一个数字是否可为uint16_t数字 */
    static bool is_uint16(int32_t num);
    static bool is_uint16(uint32_t num);

    /** 判断一个数字是否可为int32_t数字 */
    static bool is_int32(int64_t num);

    /** 判断一个数字是否可为uint32_t数字 */
    static bool is_uint32(int64_t num);
    static bool is_uint32(uint64_t num);
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_INTEGER_UTILS_H
