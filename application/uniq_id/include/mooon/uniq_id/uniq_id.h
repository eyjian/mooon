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
    MINOR_VERSION = 4  // 次版本号
};

// 出错代码（不要超过int32_t取值范围）
enum
{
    ERROR_INVALID_TYPE = 201600001,   // 无效的请求消息类型
    ERROR_STORE_SEQ = 201600002,      // 保存sequence失败
    ERROR_OVERFLOW = 201600003,       // Sequence被消耗完了
    ERROR_LABEL_EXPIRED = 201600004,  // Label过期了
    ERROR_INVALID_LABEL = 201600005,  // 无效的Label，比如值过大或为0或为负
    ERROR_NO_LABEL = 201600006,       // Label被租光了
    ERROR_LABEL_NOT_HOLD = 201600007, // Label被其它租用着
    ERROR_DATABASE = 201600008,       // DB错误
    ERROR_PARAMETER = 201600009,      // 参数错误
    ERROR_MISMATCH = 201600010,       // 不匹配的响应
    ERROR_UNEXCEPTED = 201600011,     // 非期望的响应，响应来自非请求的Agent
    ERROR_ILLEGAL = 201600012         // 非法的数据包
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
        uint64_t hour:5;   // 当前的小时数
        uint64_t seq:29;   // 循环递增的序列号，最大为536870911，注意1小时内的数量不能超过这个值，否则会重！

        std::string str() const
        {
            return mooon::utils::CStringUtils::format_string(
                    "uniq://U%d/L%02X/%d-%d-%d_%d/S%u",
                    static_cast<int>(user), static_cast<int>(label),
                    static_cast<int>(year+BASE_YEAR), static_cast<int>(month), static_cast<int>(day),
                    static_cast<int>(hour), static_cast<unsigned int>(seq));
        }
    }id;
};

const char* label2string(uint8_t label, char str[3], bool uppercase=true);
std::string label2string(uint8_t label, bool uppercase=true);

class CUniqId
{
public:
    // agent_nodes 以逗号分隔的agent节点字符串，如：192.168.31.21:6200,192.168.31.22:6200,192.168.31.23:6200
    // timeout_milliseconds 接收agent返回超时值
    // retry_times 从一个agent取失败时，改从多少其它agent取，如果值为0表示不重试
    // polling 是否轮询取agent，效率会比随机高一点
    // 出错抛 mooon::utils::CException 异常
    CUniqId(const std::string& agent_nodes, uint32_t timeout_milliseconds=300, uint8_t retry_times=3, bool polling=false);
    ~CUniqId();

    // 取得机器Label（标签），用于唯一区分机器，同一时间两台机器不会出现相同的Label
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    uint8_t get_label();

    // 获取seq，
    // 参数用来指定获取连续的num个，返回值为起始的seq，
    // 参数num值为0或1均表示只取一个，否则取num指定的个数
    //
    // UniqAgent的steps参数值不能比num值小，最好是num的10倍或以上
    //
    // 返回的seq值最大为4294967295，即无符号4字节整数的最大值，最小值为1
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    uint32_t get_unqi_seq(uint16_t num=1);

    // 取得一个唯一的无符号8字节的整数，可用来唯一标识一个消息等
    // current_seconds 通常为time(NULL)的返回值，user可以为用户定义的值，但最大只能为63
    //                 函数实现会取s的年份、月份、天和小时，具体可以参考UniqID的定义。
    // 由于seq一小时内只有10亿的容量，如果不够用，则可以将分钟设置到user参数，这样就扩容1分钟10亿的容量。
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    uint64_t get_uniq_id(uint8_t user=0, uint64_t current_seconds=0);

    // 和get_uniq_id的区别在于，get_local_uniq_id只从agent取得Label和Seq，组装是在本地完成的，相当于分担了agent的部分工作。
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    uint64_t get_local_uniq_id(uint8_t user=0, uint64_t current_seconds=0);

    // UniqAgent的steps参数值，不能比num值小，最好是num的10倍或以上
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    void get_local_uniq_id(uint16_t num, std::vector<uint64_t>* id_vec, uint8_t user=0, uint64_t current_seconds=0);

    // 同时取得机器Label和seq值，可用这两者来组装交易流水号等
    // UniqAgent的steps参数值不能比num值小，最好是num的10倍或以上
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    void get_label_and_seq(uint8_t* label, uint32_t* seq, uint16_t num=1);

    // 取流水号、交易号等便利函数
    // format 取值：
    //   %Y 年份 4位数字，如：2016
    //   %M 月份 2位数字，以0填充，如：11月为11，9月为09
    //   %D 日期 2位数字，以0填充
    //   %H 小时 2位数字，以0填充
    //   %m 分钟 2位数字，以0填充
    //   %S Sequence，一个4字节无符号整数值，可指定宽度，但总是以0填充
    //   %L Label 机器标签，输出为2个十六进制字符
    //   %d 4字节十进制整数，可指定宽度，但总是以0填充
    //   %s 字符串
    //   %X 十六进制，可指定宽度，但总是以0填充
    //
    // 注意，只有%S、%d和%X有宽度参数，如：%4S%d，并且不足时统一填充0，不能指定填充数字，而且宽长参数不能超过9
    // 使用示例：%9S, %2d, %5X，不能为%09S、%02d和%05X等，%10S等超过9的宽度是无效的。
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    std::string get_transaction_id(const char* format, ...);
    std::string get_transaction_id(const char* format, va_list& va);

    // UniqAgent的steps参数值不能比num值小，最好是num的10倍或以上
    // 出错抛 mooon::utils::CException,mooon::sys::CSyscallException 异常
    void get_transaction_id(uint16_t num, std::vector<std::string>* id_vec, const char* format, ...);
    void get_transaction_id(uint16_t num, std::vector<std::string>* id_vec, const char* format, va_list& va);

private:
    const struct sockaddr_in& pick_agent() const;

private:
    uint32_t _echo;
    const std::string& _agent_nodes;
    uint32_t _timeout_milliseconds;
    uint8_t _retry_times;
    bool _polling; // 是否轮询选择UniqAgent，否则随机方式，轮询方式选择开销小
    std::vector<struct sockaddr_in> _agents_addr;
    net::CUdpSocket* _udp_socket;
};

} // namespace mooon {
#endif // MOOON_UNIQ_ID_H
