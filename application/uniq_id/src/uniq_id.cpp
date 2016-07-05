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
#include <mooon/utils/tokener.h>
#include <mooon/utils/string_utils.h>
#include <mooon/sys/utils.h>
#include <sstream>
#include <sys/time.h>
#include <time.h>
namespace mooon {

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

CUniqId::CUniqId(const std::string& agent_nodes, uint32_t timeout_milliseconds, uint8_t retry_times, bool polling) throw (utils::CException)
    : _echo(0), _agent_nodes(agent_nodes), _timeout_milliseconds(timeout_milliseconds), _retry_times(retry_times), _polling(polling), _udp_socket(NULL)
{
    _echo = sys::CUtils::get_random_number(0, 100000U); // 初始化一个随机值，这样不同实例不同
    _udp_socket = new net::CUdpSocket;

    utils::CEnhancedTokener tokener;
    tokener.parse(agent_nodes, ",", ':');

    const std::map<std::string, std::string>& tokens = tokener.tokens();
    for (std::map<std::string, std::string>::const_iterator iter=tokens.begin(); iter!=tokens.end(); ++iter)
    {
        uint16_t agent_port;
        if (!utils::CStringUtils::string2int(iter->second.c_str(), agent_port))
        {
            THROW_EXCEPTION("invalid port parameter", ERROR_PARAMETER);
        }

        struct sockaddr_in agent_addr;
        agent_addr.sin_addr.s_addr = inet_addr(iter->first.c_str());
        if (INADDR_NONE == agent_addr.sin_addr.s_addr)
        {
            THROW_EXCEPTION("invalid IP parameter", ERROR_PARAMETER);
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

uint8_t CUniqId::get_label() throw (utils::CException, sys::CSyscallException)
{
    uint32_t echo = _echo++;
    struct MessageHead response;
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_LABEL;
    request.echo = echo;
    request.value1 = 0;
    request.value2 = 0;
    request.value3 = 0;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        try
        {
            struct sockaddr_in from_addr;
            const struct sockaddr_in& agent_addr = pick_agent();
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

            bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(response))
            {
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");
            }
            else if (RESPONSE_ERROR == response.type)
            {
                THROW_EXCEPTION("store sequence block error", static_cast<int>(response.value1.to_int()));
            }
            else if (response.type != RESPONSE_LABEL)
            {
                THROW_EXCEPTION("error response label", response.type.to_int());
            }
            else if (response.echo.to_int() != echo)
            {
                THROW_EXCEPTION("mismatch response label", ERROR_MISMATCH);
            }
            else
            {
                return static_cast<uint8_t>(response.value1.to_int());
            }
        }
        catch (utils::CException&)
        {
            // 在重试之前不抛出异常
            if ((retry == _retry_times) || (0 == _retry_times))
                throw;
        }
    }

    return 0;
}

uint32_t CUniqId::get_unqi_seq(uint16_t num) throw (utils::CException, sys::CSyscallException)
{
    uint32_t echo = _echo++;
    struct MessageHead response;
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_UNIQ_SEQ;
    request.echo = echo;
    request.value1 = num;
    request.value2 = 0;
    request.value3 = 0;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        try
        {
            struct sockaddr_in from_addr;
            const struct sockaddr_in& agent_addr = pick_agent();
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

            bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(response))
            {
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");
            }
            else if (RESPONSE_ERROR == response.type)
            {
                THROW_EXCEPTION("store sequence block error", static_cast<int>(response.value1.to_int()));
            }
            else if (response.type != RESPONSE_UNIQ_SEQ)
            {
                THROW_EXCEPTION("error response sequence", response.type.to_int());
            }
            else if (response.echo.to_int() != echo)
            {
                THROW_EXCEPTION("mismatch response sequence", ERROR_MISMATCH);
            }
            else
            {
                return static_cast<uint32_t>(response.value1.to_int());
            }
        }
        catch (utils::CException&)
        {
            if ((retry == _retry_times) || (0 == _retry_times))
                throw;
        }
    }

    return 0;
}

uint64_t CUniqId::get_uniq_id(uint8_t user, uint64_t current_seconds) throw (utils::CException, sys::CSyscallException)
{
    uint32_t echo = _echo++;
    struct MessageHead response;
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_UNIQ_ID;
    request.echo = echo;
    request.value1 = user;
    request.value1 = 0;
    request.value3 = current_seconds;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        try
        {
            struct sockaddr_in from_addr;
            const struct sockaddr_in& agent_addr = pick_agent();
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

            bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(response))
            {
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");
            }
            else if (RESPONSE_ERROR == response.type)
            {
                THROW_EXCEPTION("store sequence block error", static_cast<int>(response.value1.to_int()));
            }
            else if (response.type != RESPONSE_UNIQ_ID)
            {
                THROW_EXCEPTION("error response id", response.type.to_int());
            }
            else if (response.echo.to_int() != echo)
            {
                THROW_EXCEPTION("mismatch response id", ERROR_MISMATCH);
            }
            else
            {
                return response.value3.to_int();
            }
        }
        catch (utils::CException&)
        {
            if ((retry == _retry_times) || (0 == _retry_times))
                throw;
        }
    }

    return 0;
}

uint64_t CUniqId::get_local_uniq_id(uint8_t user, uint64_t current_seconds) throw (utils::CException, sys::CSyscallException)
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

void CUniqId::get_local_uniq_id(uint16_t num, std::vector<uint64_t>* id_vec, uint8_t user, uint64_t current_seconds) throw (utils::CException, sys::CSyscallException)
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

void CUniqId::get_label_and_seq(uint8_t* label, uint32_t* seq, uint16_t num) throw (utils::CException, sys::CSyscallException)
{
    uint32_t echo = _echo++;
    struct MessageHead response;
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_LABEL_AND_SEQ;
    request.echo = echo;
    request.value1 = num;
    request.value2 = 0;
    request.value3 = 0;

    for (uint8_t retry=0; retry<_retry_times+1; ++retry)
    {
        try
        {
            struct sockaddr_in from_addr;
            const struct sockaddr_in& agent_addr = pick_agent();
            int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
            if (bytes != sizeof(request))
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

            bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
            if (bytes != sizeof(response))
            {
                THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");
            }
            else if (RESPONSE_ERROR == response.type)
            {
                THROW_EXCEPTION("store sequence block error", static_cast<int>(response.value1.to_int()));
            }
            else if (response.type != RESPONSE_LABEL_AND_SEQ)
            {
                THROW_EXCEPTION("error response label and sequence", response.type.to_int());
            }
            else if (response.echo.to_int() != echo)
            {
                THROW_EXCEPTION("mismatch response label and sequence", ERROR_MISMATCH);
            }
            else
            {
                *label = static_cast<uint8_t>(response.value1.to_int());
                *seq = static_cast<uint32_t>(response.value2.to_int());
                break;
            }
        }
        catch (utils::CException&)
        {
            if ((retry == _retry_times) || (0 == _retry_times))
                throw;
        }
    }
}

// %Y 年份 %M 月份 %D 日期 %H 小时 %m 分钟 %S Sequence %L Label %d 4字节十进制整数 %s 字符串 %X 十六进制
// 只有%S和%d有宽度参数，如：%4S%d，并且不足时统一填充0，不能指定填充数字
std::string CUniqId::get_transaction_id(const char* format, ...) throw (utils::CException, sys::CSyscallException)
{
    va_list ap;
    va_start(ap, format);
    utils::VaListHelper vlh(ap);

    return get_transaction_id(format, ap);
}

std::string CUniqId::get_transaction_id(const char* format, va_list& va) throw (utils::CException, sys::CSyscallException)
{
    std::vector<std::string> id_vec;
    get_transaction_id(1, &id_vec, format, va);

    if (!id_vec.empty())
        return id_vec[0];
    else
        return std::string("");
}

void CUniqId::get_transaction_id(uint16_t num, std::vector<std::string>* id_vec, const char* format, ...) throw (utils::CException, sys::CSyscallException)
{
    va_list ap;
    va_start(ap, format);
    utils::VaListHelper vlh(ap);

    get_transaction_id(num, id_vec, format, ap);
}

void CUniqId::get_transaction_id(uint16_t num, std::vector<std::string>* id_vec, const char* format, va_list& va) throw (utils::CException, sys::CSyscallException)
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
                    THROW_EXCEPTION("invalid `format` parameter", ERROR_PARAMETER);
                }
                else
                {
                    if ((*format_p >= '0') && (*format_p <= '9'))
                    {
                        width = *format_p - '0';
                        ++format_p; // 跳过width

                        if ('S' == *format_p) // Sequence
                        {
                            result << std::setw(width) << std::setfill('0') << seq;
                        }
                        else if ('d' == *format_p) // integer
                        {
                            m = va_arg(ap, int);
                            result << std::dec << std::setw(width) << std::setfill('0') << m;
                        }
                        else if ('X' == *format_p)
                        {
                            m = va_arg(ap, int);
                            result << std::hex << std::setw(width) << std::setfill('0') << std::uppercase << m;
                        }
                        else
                        {
                            // format error
                            THROW_EXCEPTION("invalid `format` parameter", ERROR_PARAMETER);
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
                            THROW_EXCEPTION("invalid `format` parameter", ERROR_PARAMETER);
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
