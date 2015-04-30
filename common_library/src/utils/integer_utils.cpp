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
#include <limits>
#include "utils/integer_utils.h"
UTILS_NAMESPACE_BEGIN

bool CIntegerUtils::is_int16(int32_t num)
{    
    return (num >= std::numeric_limits<int16_t>::min())
        && (num <= std::numeric_limits<int16_t>::max());
}

bool CIntegerUtils::is_uint16(int32_t num)
{
    return (num >= 0) && (num <= std::numeric_limits<uint16_t>::max());
}

bool CIntegerUtils::is_uint16(uint32_t num)
{
    return (num <= std::numeric_limits<uint16_t>::max());
}

bool CIntegerUtils::is_int32(int64_t num)
{
    return (num >= std::numeric_limits<int32_t>::min())
        && (num <= std::numeric_limits<int32_t>::max());
}

bool CIntegerUtils::is_uint32(int64_t num)
{
    return (num >= 0) && (num <= std::numeric_limits<uint32_t>::max());
}

bool CIntegerUtils::is_uint32(uint64_t num)
{
    return (num <= std::numeric_limits<uint32_t>::max());
}

UTILS_NAMESPACE_END
