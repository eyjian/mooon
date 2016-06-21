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
#include <mooon/net/inttypes.h>
#include <mooon/utils/string_utils.h>
namespace mooon {

// 常量
enum
{
    SOCKET_BUFFER_SIZE = 1024,
    LABEL_MAX = 255,                    // Label最大的取值，注意只能为1023，不能为更大的值
    LABEL_EXPIRED_SECONDS = (3600*24*7) // Label多少小秒过期，默认7天
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
    nuint32_t echo; // 响应时回复相同的值
    nuint64_t value1;
    nuint64_t value2;

    std::string str() const
    {
        return utils::CStringUtils::format_string("message://%d/%d/%u/%"PRIu64"/%"PRIu64, len.to_int(), type.to_int(), echo.to_int(), value1.to_int(), value2.to_int());
    }
};

#pragma pack()

} // namespace mooon {
#endif // MOOON_UNIQ_ID_PROTOCOL_H
