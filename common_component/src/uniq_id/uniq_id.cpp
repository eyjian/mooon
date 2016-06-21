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
#include <mooon/net/udp_socket.h>
#include <mooon/utils/tokener.h>
#include <mooon/utils/string_utils.h>
#include <time.h>
namespace mooon {

CUniqId::CUniqId(const std::string& agent_nodes, uint32_t timeout_milliseconds) throw (utils::CException)
    : _echo(0), _agent_nodes(agent_nodes), _timeout_milliseconds(timeout_milliseconds), _udp_socket(NULL)
{
    _udp_socket = new net::CUdpSocket;

    utils::CEnhancedTokener tokener;
    tokener.parse(agent_nodes, ",", ':');

    const std::map<std::string, std::string>& tokens = tokener.tokens();
    for (std::map<std::string, std::string>::const_iterator iter=tokens.begin(); iter!=tokens.end(); ++iter)
    {
        uint16_t agent_port;
        if (!utils::CStringUtils::string2int(iter->second.c_str(), agent_port))
        {
            THROW_EXCEPTION("invalid port", -1);
        }

        struct sockaddr_in agent_addr;
        agent_addr.sin_addr.s_addr = inet_addr(iter->first.c_str());
        if (INADDR_NONE == agent_addr.sin_addr.s_addr)
        {
            THROW_EXCEPTION("invalid IP", -1);
        }

        agent_addr.sin_family = AF_INET;
        agent_addr.sin_port = htons(agent_port);
        memset(agent_addr.sin_zero, 0, sizeof(agent_addr.sin_zero));
        agents_addr.push_back(agent_addr);
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

    struct sockaddr_in from_addr;
    const struct sockaddr_in& agent_addr = pick_agent();
    int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
    if (bytes != sizeof(request))
        THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

    bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
    if (bytes != sizeof(response))
        THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");

    if (response.echo.to_int() == echo)
        return response.value1;
    return 0;
}

uint32_t CUniqId::get_unqi_seq() throw (utils::CException, sys::CSyscallException)
{
    uint32_t echo = _echo++;
    struct MessageHead response;
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_UNIQ_SEQ;
    request.echo = echo;
    request.value1 = 0;
    request.value2 = 0;

    struct sockaddr_in from_addr;
    const struct sockaddr_in& agent_addr = pick_agent();
    int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
    if (bytes != sizeof(request))
        THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

    bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
    if (bytes != sizeof(response))
        THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");

    if (response.echo.to_int() == echo)
        return response.value1;
    return 0;
}

uint64_t CUniqId::get_uniq_id(uint8_t user, uint64_t s) throw (utils::CException, sys::CSyscallException)
{
    uint32_t echo = _echo++;
    struct MessageHead response;
    struct MessageHead request;
    request.len = sizeof(request);
    request.type = REQUEST_UNIQ_ID;
    request.echo = echo;
    request.value1 = user;
    request.value2 = s;

    struct sockaddr_in from_addr;
    const struct sockaddr_in& agent_addr = pick_agent();
    int bytes = _udp_socket->send_to(&request, sizeof(request), agent_addr);
    if (bytes != sizeof(request))
        THROW_SYSCALL_EXCEPTION("invalid size", bytes, "send_to");

    bytes = _udp_socket->timed_receive_from(&response, sizeof(response), &from_addr, _timeout_milliseconds);
    if (bytes != sizeof(response))
        THROW_SYSCALL_EXCEPTION("invalid size", bytes, "receive_from");
    else if (RESPONSE_ERROR == response.type)
        THROW_EXCEPTION("store sequence block error", static_cast<int>(response.value1.to_int()));
    else if (response.echo.to_int() == echo)
        return response.value1;
    return 0;
}

const struct sockaddr_in& CUniqId::pick_agent() const
{
    MOOON_ASSERT(!agents_addr.empty());

    static uint64_t i = 0;
    return agents_addr[i % static_cast<uint64_t>(agents_addr.size())];
}

} // namespace mooon {
