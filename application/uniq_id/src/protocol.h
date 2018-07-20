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
#ifndef MOOON_UNIQ_ID_PROTOCOL_H
#define MOOON_UNIQ_ID_PROTOCOL_H
#include "crc32.h"
#include "mooon/uniq_id/uniq_id.h"
#include <mooon/net/inttypes.h>
#include <mooon/utils/string_utils.h>
namespace mooon {

// 常量
enum
{
    // 多数设备以太网链路层MTU（最大传输单元）大小为1500（但标准为576），IP包头为20，TCP包头为20，UDP包头8
    //
    // 1500为链路层的数据大小，不包括链路层首部和尾部的18个字节，这1500实际为网络层（IP层）的数据大小
    // IP首部为20字节，所以IP数据大小为1480，
    // UDP首部8字节，所以UDP数据大小为1472，当UDP发送大小1472的数据时，链路层需要分片（Fragment），这样IP层需要重组数据，可能会重组失败导致数据丢失
    //
    // 标准的MTU大小为576（Windows默认为1500），减去IP首部和UDP首部后为548，因此UDP发送的数据大小不超过548是最安全的。
    SOCKET_BUFFER_SIZE = 128,
    LABEL_MAX = 254,                      // Label最大的取值（不包含0，从1开始），注意只能为254，不能为更大的值
    LABEL_EXPIRED_SECONDS = (3600*24*15), // Label多少小秒过期，默认15天
    ECHO_START = 1357, // echo起始值，为0容易恰好碰上
    RETRY_MAX = 128 // 最多重试次数，如果超过则会置为128
};

// 命令字
enum
{
    REQUEST_LABEL = 1,
    REQUEST_UNIQ_ID = 2,
    REQUEST_UNIQ_SEQ = 3,
    REQUEST_LABEL_AND_SEQ = 4,

    RESPONSE_ERROR = 100,
    RESPONSE_LABEL = 101,
    RESPONSE_UNIQ_ID = 102,
    RESPONSE_UNIQ_SEQ = 103,
    RESPONSE_LABEL_AND_SEQ = 104
};

////////////////////////////////////////////////////////////////////////////////
#pragma pack(4)

// 请求和响应共用的消息头
struct MessageHead
{
    nuint16_t len;
    nuint16_t type;
    nuint16_t major_ver; // 主版本号
    nuint16_t minor_ver; // 次版本号
    nuint32_t magic;     // 有来校验包
    nuint32_t echo;      // 响应时回复相同的值
    nuint32_t value1;    // 当type值为RESPONSE_UNIQ_SEQ时，value1表示连续取多少个seq
    nuint32_t value2;
    nuint64_t value3;    // 当type值为RESPONSE_UNIQ_SEQ时，value2，表示从value1开始连续多少个seq，但0为无效的seq

    MessageHead()
        : major_ver(MAJOR_VERSION), minor_ver(MINOR_VERSION)
    {
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string("message://V:%d.%d/L:%d/T:%d/E:%u/M:%u/V1:%u/V2:%u/V3:%" PRIu64, (int)major_ver.to_int(), (int)minor_ver.to_int(), (int)len.to_int(), (int)type.to_int(), echo.to_int(), magic.to_int(), value1.to_int(), value2.to_int(), value3.to_int());
    }

    uint32_t calc_magic() const
    {
        const uint16_t len_ = len.to_int();
        const uint16_t type_ = type.to_int();
        const uint16_t major_ver_ = major_ver.to_int();
        const uint16_t minor_ver_ = minor_ver.to_int();
        const uint32_t echo_ = echo.to_int();
        const uint32_t value1_ = value1.to_int();
        const uint32_t value2_ = value2.to_int();
        const uint64_t value3_ = value3.to_int();
        uint32_t magic = 0;

        magic = crc32(magic, &len_, sizeof(len_));
        magic = crc32(magic, &type_, sizeof(type_));
        magic = crc32(magic, &major_ver_, sizeof(major_ver_));
        magic = crc32(magic, &minor_ver_, sizeof(minor_ver_));
        magic = crc32(magic, &echo_, sizeof(echo_));
        magic = crc32(magic, &value1_, sizeof(value1_));
        magic = crc32(magic, &value2_, sizeof(value2_));
        magic = crc32(magic, &value3_, sizeof(value3_));
        return magic;
    }

    void update_magic()
    {
        magic = calc_magic();
    }
};

#pragma pack()

} // namespace mooon {
#endif // MOOON_UNIQ_ID_PROTOCOL_H
