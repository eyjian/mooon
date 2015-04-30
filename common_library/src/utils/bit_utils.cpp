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
#include "utils/bit_utils.h"
UTILS_NAMESPACE_BEGIN

void CBitUtils::flip(char* bitmap, uint32_t position)
{
    bitmap[position / 8] ^= 1 << (position % 8);
}

bool CBitUtils::test(char* bitmap, uint32_t position)
{
    return 1 == get_bit(bitmap, position);
}

uint8_t CBitUtils::get_bit(char* bitmap, uint32_t position)
{
    return bitmap[position / 8] >> (position % 8) & 1;
}

void CBitUtils::set_bit(char* bitmap, uint32_t position, bool zero)
{    
    if (zero)
    {
        bitmap[position / 8] &= ~(1 << (position % 8));
    }
    else
    {
        bitmap[position / 8] |= 1 << (position % 8);
    }
}

UTILS_NAMESPACE_END
