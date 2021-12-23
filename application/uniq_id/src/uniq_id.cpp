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
#include "mooon/uniq_id/uniq_id.h"
#include "protocol.h"
#include <iomanip>
#include <mooon/net/udp_socket.h>
#include <mooon/utils/integer_utils.h>
#include <mooon/utils/tokener.h>
#include <mooon/utils/string_utils.h>
#include <mooon/sys/utils.h>
#include <sstream>
#include <sys/time.h>
#include <time.h>

// 是否检查magic
#define _CHECK_MAGIC_ 1

namespace mooon {

// 尽量避免容易碰撞的echo值
static uint32_t get_echo(uint32_t echo)
{
    uint32_t echo_ = echo;

    if (echo_ < ECHO_START)
    {
        echo_ = ECHO_START + sys::CUtils::get_random_number(0, 1235U);
    }
    else if (0 == echo_ % 10)
    {
        echo_ = echo_ + 1;
    }

    return echo_;
}

const char* label2string(uint8_t label, char str[3], bool uppercase)
{
    if (uppercase)
        snprintf(str, 3, "%02X", label);
    else
        snprintf(str, 3, "%02x", label);
    return str;
}

std::string label2string(uint8_t label, bool uppercase)
{
    char str[3];
    label2string(label, str, uppercase);
    return str;
}

CUniqId::CUniqId(const std::string& agent_nodes, uint32_t timeout_milliseconds, uint8_t retry_times, bool polling)
    : _echo(ECHO_START), _agent_nodes(agent_nodes), _timeout_milliseconds(timeout_milliseconds),
      _retry_times(retry_times), _polling(polling), _udp_socket(NULL)
{
    _udp_socket = new net::CUdpSocket;
    _echo = ECHO_START + sys::CUtils::get_random_number(0, 1235U); // 初始化一个随机值，这样不同实例不同
    _echo = get_echo(_echo);

    // 限制最大重试次数，这样可以保证后续的“retry+1”不会溢出
    if (_retry_times > RETRY_MAX)
    {
        _retry_times = RETRY_MAX;
    }

    utils::CEnhancedTokenerEx tokener;
    tokener.parse(agent_nodes, ",", ':');

    const std::multimap<std::string, std::string>& tokens = tokener.tokens();
    for (std::multimap<std::string, std::string>::const_iterator iter=tokens.begin(); iter!=tokens.end(); ++iter)
    {
        uint16_t agent_port;
        if (!utils::CStringUtils::string2int(iter->second.c_str(), agent_port))
        {
            THROW_EXCEPTION("[UniqID] invalid port parameter", ERROR_PARAMETER);
        }

        struct sockaddr_in agent_addr;
        agent_addr.sin_addr.s_addr = inet_addr(iter->first.c_str());
        if (INADDR_NONE == agent_addr.sin_addr.s_addr)
        {
            THROW_EXCEPTION("[UniqID] invalid IP parameter", ERROR_PARAMETER);
        }

        agent_addr.sin_family = AF_INET;
        agent_addr.sin_port = htons(agent_port);
        memset(agent_addr.sin_zero, 0, sizeof(agent_addr.sin_zero));
        _agents_addr.push_back(agent_addr);
    }
}

CUniqId::~CUniqId()
{
    delete _udp_socket;
}

uint8_t CUniqId::get_label()
{
    const uint32_t echo = get_echo(_echo);
    char response_buffer[1 + sizeof(struct MessageHead)]; // 故意多出一字节，以过滤掉包大小不同的脏数据
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(response_buffer);
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_LABEL;
    request.echo = echo;
    request.value1 = 0;
    request.value2 = 0;
    request.value3 = 0;
    request.update_magic();
    _echo = echo + 1;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        const struct sockaddr_in& agent_addr = pick_agent();

        try
        {
            struct sockaddr_in from_addr;
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(agent_addr).c_str()),
                        bytes, "send_to");
            }

            bytes = _udp_socket->timed_receive_from(response, sizeof(response_buffer), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(struct MessageHead))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(from_addr).c_str()),
                        bytes, "receive_from");
            }
            else if (memcmp(&agent_addr, &from_addr, sizeof(struct sockaddr_in)) != 0)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s][AGENT:%s] unexcepted response",
                                net::to_string(from_addr).c_str(), net::to_string(agent_addr).c_str()),
                        ERROR_UNEXCEPTED);
            }
            else if (RESPONSE_ERROR == response->type)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] store sequence block error: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        static_cast<int>(response->value1.to_int()));
            }
            else if (response->type != RESPONSE_LABEL)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] error response label: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        response->type.to_int());
            }
            else if (response->echo.to_int() != echo)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] mismatch response label: %s|%u",
                                net::to_string(from_addr).c_str(), response->str().c_str(), echo),
                        ERROR_MISMATCH);
            }
            else
            {
#if _CHECK_MAGIC_ == 1
                const uint32_t magic_ = response->calc_magic();
                //fprintf(stderr, "%s|%u\n", response->str().c_str(), magic_);
                if (magic_ != response->magic)
                {
                    THROW_EXCEPTION(
                            utils::CStringUtils::format_string("[UniqID][%s] illegal response: %s|%u",
                                    net::to_string(from_addr).c_str(), response->str().c_str(), magic_),
                            ERROR_ILLEGAL);
                }
#endif // _CHECK_MAGIC_

                const uint32_t label_ = response->value1.to_int();
                if ((label_ >= 0xFF) || (label_ < 1))
                {
                    THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid label from master: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        ERROR_INVALID_LABEL);
                }

                return static_cast<uint8_t>(label_);
            }
        }
        catch (sys::CSyscallException& ex)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
            {
                if (ex.errcode() != ETIMEDOUT)
                    throw;

                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] receive timeout", net::to_string(agent_addr).c_str()),
                        ETIMEDOUT, "timed_receive_from");
            }
        }
        catch (utils::CException&)
        {
            // 在重试之前不抛出异常
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
                throw;
        }
    }

    return 0;
}

uint32_t CUniqId::get_unqi_seq(uint16_t num)
{
    const uint32_t echo = get_echo(_echo);
    char response_buffer[1 + sizeof(struct MessageHead)]; // 故意多出一字节，以过滤掉包大小不同的脏数据
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(response_buffer);
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_UNIQ_SEQ;
    request.echo = echo;
    request.value1 = num;
    request.value2 = 0;
    request.value3 = 0;
    request.update_magic();
    _echo = echo + 1;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        const struct sockaddr_in& agent_addr = pick_agent();

        try
        {
            struct sockaddr_in from_addr;
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(agent_addr).c_str()),
                        bytes, "send_to");

            bytes = _udp_socket->timed_receive_from(response, sizeof(response_buffer), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(struct MessageHead))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(from_addr).c_str()),
                        bytes, "receive_from");
            }
            else if (memcmp(&agent_addr, &from_addr, sizeof(struct sockaddr_in)) != 0)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s][AGENT:%s] unexcepted response",
                                net::to_string(from_addr).c_str(), net::to_string(agent_addr).c_str()),
                        ERROR_UNEXCEPTED);
            }
            else if (RESPONSE_ERROR == response->type)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] store sequence block error: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        static_cast<int>(response->value1.to_int()));
            }
            else if (response->type != RESPONSE_UNIQ_SEQ)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] error response sequence: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        response->type.to_int());
            }
            else if (response->echo.to_int() != echo)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] mismatch response sequence: %s|%u",
                                net::to_string(from_addr).c_str(), response->str().c_str(), echo),
                        ERROR_MISMATCH);
            }
            else
            {
#if _CHECK_MAGIC_ == 1
                const uint32_t magic_ = response->calc_magic();
                //fprintf(stderr, "%s|%u\n", response->str().c_str(), magic_);
                if (magic_ != response->magic)
                {
                    THROW_EXCEPTION(
                            utils::CStringUtils::format_string("[UniqID][%s] illegal response: %s|%u",
                                    net::to_string(from_addr).c_str(), response->str().c_str(), magic_),
                            ERROR_ILLEGAL);
                }
#endif // _CHECK_MAGIC_

                return static_cast<uint32_t>(response->value1.to_int());
            }
        }
        catch (sys::CSyscallException& ex)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
            {
                if (ex.errcode() != ETIMEDOUT)
                    throw;

                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] receive timeout", net::to_string(agent_addr).c_str()),
                        ETIMEDOUT, "timed_receive_from");
            }
        }
        catch (utils::CException&)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
                throw;
        }
    }

    return 0;
}

uint64_t CUniqId::get_uniq_id(uint8_t user, uint64_t current_seconds)
{
    const uint32_t echo = get_echo(_echo);
    char response_buffer[1 + sizeof(struct MessageHead)]; // 故意多出一字节，以过滤掉包大小不同的脏数据
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(response_buffer);
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_UNIQ_ID;
    request.echo = echo;
    request.value1 = user;
    request.value1 = 0;
    request.value3 = current_seconds;
    request.update_magic();
    _echo = echo + 1;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        const struct sockaddr_in& agent_addr = pick_agent();

        try
        {
            struct sockaddr_in from_addr;
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(agent_addr).c_str()),
                        bytes, "send_to");
            }

            bytes = _udp_socket->timed_receive_from(response, sizeof(response_buffer), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(struct MessageHead))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(from_addr).c_str()),
                        bytes, "receive_from");
            }
            else if (memcmp(&agent_addr, &from_addr, sizeof(struct sockaddr_in)) != 0)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s][AGENT:%s] unexcepted response",
                                net::to_string(from_addr).c_str(), net::to_string(agent_addr).c_str()),
                        ERROR_UNEXCEPTED);
            }
            else if (RESPONSE_ERROR == response->type)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] store sequence block error: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        static_cast<int>(response->value1.to_int()));
            }
            else if (response->type != RESPONSE_UNIQ_ID)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] error response id: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        response->type.to_int());
            }
            else if (response->echo.to_int() != echo)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] mismatch response id: %s|%u",
                                net::to_string(from_addr).c_str(), response->str().c_str(), echo),
                        ERROR_MISMATCH);
            }
            else
            {
#if _CHECK_MAGIC_ == 1
                const uint32_t magic_ = response->calc_magic();
                //fprintf(stderr, "%s|%u\n", response->str().c_str(), magic_);
                if (magic_ != response->magic)
                {
                    THROW_EXCEPTION(
                            utils::CStringUtils::format_string("[UniqID][%s] illegal response: %s|%u",
                                    net::to_string(from_addr).c_str(), response->str().c_str(), magic_),
                            ERROR_ILLEGAL);
                }
#endif // _CHECK_MAGIC_

                return response->value3.to_int();
            }
        }
        catch (sys::CSyscallException& ex)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
            {
                if (ex.errcode() != ETIMEDOUT)
                    throw;

                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] receive timeout", net::to_string(agent_addr).c_str()),
                        ETIMEDOUT, "timed_receive_from");
            }
        }
        catch (utils::CException&)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
                throw;
        }
    }

    return 0;
}

uint64_t CUniqId::get_local_uniq_id(uint8_t user, uint64_t current_seconds)
{
    uint8_t label = 0;
    uint32_t seq = 0;
    get_label_and_seq(&label, &seq);

    struct tm now;
    time_t current_time = (0 == current_seconds)? time(NULL): current_seconds;
    localtime_r(&current_time, &now);

    union UniqID uniq_id;
    uniq_id.id.user = user;
    uniq_id.id.label = label;
    uniq_id.id.year = (now.tm_year+1900) - BASE_YEAR;
    uniq_id.id.month = now.tm_mon+1;
    uniq_id.id.day = now.tm_mday;
    uniq_id.id.hour = now.tm_hour;
    uniq_id.id.seq = seq;

    return uniq_id.value;
}

void CUniqId::get_local_uniq_id(uint16_t num, std::vector<uint64_t>* id_vec, uint8_t user, uint64_t current_seconds)
{
    uint8_t label = 0;
    uint32_t seq = 0;
    get_label_and_seq(&label, &seq, num);

    struct tm now;
    time_t current_time = (0 == current_seconds)? time(NULL): current_seconds;
    localtime_r(&current_time, &now);

    union UniqID uniq_id;
    uniq_id.id.user = user;
    uniq_id.id.label = label;
    uniq_id.id.year = (now.tm_year+1900) - BASE_YEAR;
    uniq_id.id.month = now.tm_mon+1;
    uniq_id.id.day = now.tm_mday;
    uniq_id.id.hour = now.tm_hour;

    for (uint16_t i=0; i<num; ++i)
    {
        uniq_id.id.seq = seq++;
        id_vec->push_back(uniq_id.value);
    }
}

void CUniqId::get_label_and_seq(uint8_t* label, uint32_t* seq, uint16_t num)
{
    const uint32_t echo = get_echo(_echo);
    char response_buffer[1 + sizeof(struct MessageHead)]; // 故意多出一字节，以过滤掉包大小不同的脏数据
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(response_buffer);
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_LABEL_AND_SEQ;
    request.echo = echo;
    request.value1 = num;
    request.value2 = 0;
    request.value3 = 0;
    request.update_magic();
    _echo = echo + 1;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        const struct sockaddr_in& agent_addr = pick_agent();

        try
        {
            struct sockaddr_in from_addr;
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(agent_addr).c_str()),
                        bytes, "send_to");
            }

            bytes = _udp_socket->timed_receive_from(response, sizeof(response_buffer), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(struct MessageHead))
            {
                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid size", net::to_string(from_addr).c_str()),
                        bytes, "receive_from");
            }
            else if (memcmp(&agent_addr, &from_addr, sizeof(struct sockaddr_in)) != 0)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s][AGENT:%s] unexcepted response",
                                net::to_string(from_addr).c_str(), net::to_string(agent_addr).c_str()),
                        ERROR_UNEXCEPTED);
            }
            else if (RESPONSE_ERROR == response->type)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] store sequence block error: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        static_cast<int>(response->value1.to_int()));
            }
            else if (response->type != RESPONSE_LABEL_AND_SEQ)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] error response label and sequence: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        response->type.to_int());
            }
            else if (response->echo.to_int() != echo)
            {
                THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] mismatch response label and sequence: %s|%u", net::to_string(from_addr).c_str(), response->str().c_str(), echo),
                        ERROR_MISMATCH);
            }
            else
            {
#if _CHECK_MAGIC_ == 1
                const uint32_t magic_ = response->calc_magic();
                //fprintf(stderr, "%s|%u\n", response->str().c_str(), magic_);
                if (magic_ != response->magic)
                {
                    THROW_EXCEPTION(
                            utils::CStringUtils::format_string("[UniqID][%s] illegal response: %s|%u",
                                    net::to_string(from_addr).c_str(), response->str().c_str(), magic_),
                            ERROR_ILLEGAL);
                }
#endif // _CHECK_MAGIC_

                const uint32_t label_ = response->value1.to_int();
                if ((label_ >= 0xFF) || (label_ < 1))
                {
                    THROW_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] invalid label from master: %s",
                                net::to_string(from_addr).c_str(), response->str().c_str()),
                        ERROR_INVALID_LABEL);
                }

                *label = static_cast<uint8_t>(label_);
                *seq = static_cast<uint32_t>(response->value2.to_int());
                break;
            }
        }
        catch (sys::CSyscallException& ex)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
            {
                if (ex.errcode() != ETIMEDOUT)
                    throw;

                THROW_SYSCALL_EXCEPTION(
                        utils::CStringUtils::format_string("[UniqID][%s] receive timeout", net::to_string(agent_addr).c_str()),
                        ETIMEDOUT, "timed_receive_from");
            }
        }
        catch (utils::CException&)
        {
            if ((0 == _retry_times) || (retry+1 >= _retry_times))
                throw;
        }
    }
}

// %Y 年份 %M 月份 %D 日期 %H 小时 %m 分钟 %S Sequence %L Label %d 4字节十进制整数 %s 字符串 %X 十六进制
// 只有%S和%d有宽度参数，如：%4S%d，并且不足时统一填充0，不能指定填充数字
std::string CUniqId::get_transaction_id(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    utils::VaListHelper vlh(ap);

    return get_transaction_id(format, ap);
}

std::string CUniqId::get_transaction_id(const char* format, va_list& va)
{
    std::vector<std::string> id_vec;
    get_transaction_id(1, &id_vec, format, va);

    if (!id_vec.empty())
        return id_vec[0];
    else
        return std::string("");
}

void CUniqId::get_transaction_id(uint16_t num, std::vector<std::string>* id_vec, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    utils::VaListHelper vlh(ap);

    get_transaction_id(num, id_vec, format, ap);
}

void CUniqId::get_transaction_id(uint16_t num, std::vector<std::string>* id_vec, const char* format, va_list& va)
{
    char *s;
    int m, width;
    uint8_t label;
    uint32_t seq;
    get_label_and_seq(&label, &seq, num);

    struct tm now;
    time_t current_time = time(NULL);
    localtime_r(&current_time, &now);

    for (uint16_t i=0; i<num; ++i, ++seq)
    {
        std::stringstream result;
        va_list ap;
        va_copy(ap, va);
        utils::VaListHelper vlh(ap);

        const char* format_p = format;
        while (*format_p != '\0')
        {
            if (*format_p != '%')
            {
                result << *format_p++;
            }
            else
            {
                ++format_p; // 跳过'%'

                if ('\0' == *format_p)
                {
                    // format error
                    THROW_EXCEPTION("[UniqID] invalid `format` parameter", ERROR_PARAMETER);
                }
                else
                {
                    if ((*format_p >= '0') && (*format_p <= '9'))
                    {
                        width = *format_p - '0';
                        ++format_p; // 跳过width

                        if ('S' == *format_p) // Sequence
                        {
                            result << std::dec << std::setw(width) << std::setfill('0') << utils::CIntegerUtils::dec_with_width(seq, width);
                        }
                        else if ('d' == *format_p) // integer
                        {
                            m = va_arg(ap, int);
                            result << std::dec << std::setw(width) << std::setfill('0') << utils::CIntegerUtils::dec_with_width(m, width);
                        }
                        else if ('X' == *format_p)
                        {
                            m = va_arg(ap, int);
                            result << std::hex << std::setw(width) << std::setfill('0') << std::uppercase << utils::CIntegerUtils::hex_with_width(m, width);
                        }
                        else
                        {
                            // format error
                            THROW_EXCEPTION("[UniqID] invalid `format` parameter", ERROR_PARAMETER);
                        }

                        ++format_p;
                    }
                    else
                    {
                        switch (*format_p)
                        {
                        case 'd': // integer
                            m = va_arg(ap, int);
                            result << std::dec << m;
                            break;
                        case 'X': // integer
                            m = va_arg(ap, int);
                            result << std::hex << std::uppercase << m;
                            break;
                        case 's': // string
                            s = va_arg(ap, char *);
                            result << s;
                            break;
                        case 'S': // Sequence
                            result << seq;
                            break;
                        case 'L': // Label
                            result << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int)label;
                            break;
                        case 'Y': // 年
                            result << std::dec << std::setw(4) << std::setfill('0') << now.tm_year+1900;
                            break;
                        case 'M': // 月
                            result << std::dec << std::setw(2) << std::setfill('0') << now.tm_mon+1;
                            break;
                        case 'D': // 天
                            result << std::dec << std::setw(2) << std::setfill('0') << now.tm_mday;
                            break;
                        case 'H': // 小时
                            result << std::dec << std::setw(2) << std::setfill('0') << now.tm_hour;
                            break;
                        case 'm': // 分钟
                            result << std::dec << std::setw(2) << std::setfill('0') << now.tm_min;
                            break;
                        default:
                            // format error
                            THROW_EXCEPTION("[UniqID] invalid `format` parameter", ERROR_PARAMETER);
                            break;
                        } // switch

                        ++format_p;
                    } // if ((*format_p >= '0') && (*format_p <= '9'))
                } // if ('\0' == *format_p)
            } // if (*format_p != '%')
        } // while

        id_vec->push_back(result.str());
    } // for
}

const struct sockaddr_in& CUniqId::pick_agent() const
{
    MOOON_ASSERT(!_agents_addr.empty());

    if (1 == _agents_addr.size())
    {
        return _agents_addr[0];
    }
    else if (_polling)
    {
        static uint32_t factor = 0;
        register uint32_t index = factor++;
        return _agents_addr[index % _agents_addr.size()];
    }
    else
    {
        static unsigned int i = 0;
        register std::vector<struct sockaddr_in>::size_type index = sys::CUtils::get_random_number(i++, _agents_addr.size());
        return _agents_addr[index];
    }
}

} // namespace mooon {
