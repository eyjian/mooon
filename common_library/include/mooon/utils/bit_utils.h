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
#ifndef MOOON_UTILS_BIT_UTILS_H
#define MOOON_UTILS_BIT_UTILS_H
#include "mooon/utils/config.h"
UTILS_NAMESPACE_BEGIN

/***
  * 位操作工具类
  */
class CBitUtils
{
public:
    /***
      * 对指定位取反，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      */
    static void flip(char* bitmap, uint32_t position);

    /***
      * 测试指定位是否为1，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      */
    static bool test(char* bitmap, uint32_t position);

    /***
      * 得到指定位的值，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      */
    static uint8_t get_bit(char* bitmap, uint32_t position);

    /***
      * 设置指定位的值，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      * @zero: 将position所在位设置为0或1，如果为true则设置为0，否则设置为1
      */
    static void set_bit(char* bitmap, uint32_t position, bool zero);
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_BIT_UTILS_H
