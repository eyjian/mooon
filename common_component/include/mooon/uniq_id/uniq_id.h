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
#ifndef MOOON_UNIQ_ID_H
#define MOOON_UNIQ_ID_H
#include <mooon/utils/exception.h>
namespace mooon {

// 64位唯一ID结构
union UniqID
{
    uint64_t value;

    struct ID
    {
        uint64_t user:6;   // 用户定义的前缀，默认为0，最大为63
        uint64_t label:10; // 机器的唯一标识，最多支持1023台机器
        uint64_t year:7;   // 当前年份，支持到2143年
        uint64_t month:4;  // 当前月份
        uint64_t day:5;    // 当前月份的天
        uint64_t hour:4;   // 当前的小时数
        uint64_t seq:28;   // 循环递增的序列号，最大为268435455
    }id;
};

uint8_t get_label() throw (utils::CException);
uint32_t get_unqi_seq() throw (utils::CException);
uint64_t get_uniq_id(uint8_t user=0, uint64_t s=0) throw (utils::CException);

} // namespace mooon {
#endif // MOOON_UNIQ_ID_H
