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
#include <mooon/net/udp_socket.h>
#include <mooon/utils/exception.h>
#include <mooon/utils/string_utils.h>
#include <stdint.h>
#include <vector>
namespace mooon {

// 常量
enum
{
    BASE_YEAR = 2016,  // 基数年份，计时开始的年份
    MAJOR_VERSION = 0, // 主版本号
    MINOR_VERSION = 1  // 次版本号
};

// 出错代码
enum
{
    ERROR_INVALID_TYPE = 201600001,   // 无效的请求消息类型
    ERROR_STORE_SEQ = 201600002,      // 保存sequence失败
    ERROR_OVERFLOW = 201600003,       // Sequence被消耗完了
    ERROR_LABEL_EXPIRED = 201600004,  // Label过期了
    ERROR_INVALID_LABEL = 201600005,  // 无效的Label，比如值过大或为0或为负
    ERROR_NO_LABEL = 201600006,       // Label被租光了
    ERROR_LABEL_NOT_HOLD = 201600007, // Label被其它租用着
    ERROR_DATABASE = 201600008        // DB错误
};

// 64位唯一ID结构
union UniqID
{
    uint64_t value;

    struct ID
    {
        uint64_t user:6;   // 用户定义的前缀，默认为0，最大为63
        uint64_t label:8;  // 机器的唯一标识，最多支持255台机器
        uint64_t year:7;   // 当前年份减去BASE_YEAR后的值，如果当前年份为2016则可支持到2143年
        uint64_t month:4;  // 当前月份
        uint64_t day:5;    // 当前月份的天
        uint64_t hour:4;   // 当前的小时数
        uint64_t seq:30;   // 循环递增的序列号，最大为1073741823

        std::string str() const
        {
            return mooon::utils::CStringUtils::format_string("uniq://%d/%02X/%d-%d-%d_%d/%u", (int)user, (int)label, (int)year+BASE_YEAR, (int)month, (int)day, (int)hour, (unsigned int)seq);
        }
    }id;
};

const char* label2string(uint8_t label, char str[3], bool uppercase=true);
std::string label2string(uint8_t label, bool uppercase=true);

class CUniqId
{
public:
    CUniqId(const std::string& agent_nodes, uint32_t timeout_milliseconds=1000) throw (utils::CException);
    ~CUniqId();

    uint8_t get_label() throw (utils::CException, sys::CSyscallException);
    uint32_t get_unqi_seq() throw (utils::CException, sys::CSyscallException);
    uint64_t get_uniq_id(uint8_t user=0, uint64_t s=0) throw (utils::CException, sys::CSyscallException);
    void get_label_and_seq(uint8_t* label, uint32_t* seq) throw (utils::CException, sys::CSyscallException);

private:
    const struct sockaddr_in& pick_agent() const;

private:
    uint32_t _echo;
    const std::string& _agent_nodes;
    uint32_t _timeout_milliseconds;
    std::vector<struct sockaddr_in> agents_addr;
    net::CUdpSocket* _udp_socket;
};

} // namespace mooon {
#endif // MOOON_UNIQ_ID_H
