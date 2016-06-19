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
#include "protocol.h"
#include <uniq_id.h>
#include <fcntl.h>
#include <mooon/net/udp_socket.h>
#include <mooon/net/utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

STRING_ARG_DEFINE(master, "", "master nodes, e.g., 192.168.31.66:2016,192.168.31.88:2016");
STRING_ARG_DEFINE(ip, "0.0.0.0", "listen IP");
INTEGER_ARG_DEFINE(uint16_t, port, 201606, 1000, 5000, "listen port");
INTEGER_ARG_DEFINE(uint16_t, label, 0, 0, 1023, "unique label of a machine");

////////////////////////////////////////////////////////////////////////////////
namespace mooon {

// 常量
enum
{
    SEQUENCE_STEPS = 1000,
    SEQUENCE_BLOCK_VERSION = 1
};

#pragma pack(4)
struct SeqBlock
{
    uint32_t version;
    uint32_t label;
    uint32_t sequence;
    uint64_t timestamp;
    uint64_t magic;

    std::string str() const
    {
        return utils::CStringUtils::format_string("block://%u/%u/%u/%"PRId64"/%"PRId64, version, label, sequence, timestamp, magic);
    }

    void update_magic()
    {
        if (timestamp >= sequence+label+version)
            magic = timestamp - (sequence+label+version);
        else
            magic = (sequence+label+version) - timestamp;
    }

    bool valid_magic() const
    {
        if (timestamp >= sequence+label+version)
            return magic == timestamp - (sequence+label+version);
        else
            return magic == (sequence+label+version) - timestamp;
    }
};
#pragma pack()

class CUniqAgent: public sys::IMainHelper
{
public:
    CUniqAgent();
    ~CUniqAgent();

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();

private:
    std::string get_sequence_path() const;
    uint32_t get_label() const;
    bool reselect_master_addr();
    bool restore_sequence();
    bool store_sequence();
    uint32_t inc_sequence();
    uint64_t get_uniq_id(const struct GetUniqIdRequest* request);

private:
    bool prepare_response_get_label();
    bool prepare_response_get_uniq_id();
    bool prepare_response_get_uniq_seq();
    bool prepare_response_get_label_and_seq();

private:
    std::vector<struct sockaddr_in> _master_addr;
    net::CUdpSocket* _udp_socket;
    uint32_t _sequence_start;
    struct SeqBlock _seq_block;
    std::string _sequence_path;
    int _sequence_fd;

private:
    struct sockaddr_in _from_addr;
    const struct MessageHead* _message_head;
    char _request_buffer[SOCKET_BUFFER_SIZE];
    char _response_buffer[SOCKET_BUFFER_SIZE];
    size_t _response_size;
};

extern "C" int main(int argc, char* argv[])
{
    CUniqAgent agent;
    return sys::main_template(&agent, argc, argv);
}

CUniqAgent::CUniqAgent()
    : _udp_socket(NULL), _sequence_fd(-1), _message_head(NULL)
{
    _sequence_start = 0;
    _sequence_path = get_sequence_path();

    memset(&_seq_block, 0, sizeof(_seq_block));
    memset(&_from_addr, 0, sizeof(_from_addr));
    memset(&_request_buffer, 0, sizeof(_request_buffer));
    memset(&_response_buffer, 0, sizeof(_response_buffer));
    _response_size = 0;
}

CUniqAgent::~CUniqAgent()
{
    delete _udp_socket;
    if (_sequence_fd != -1)
        close(_sequence_fd);
}

bool CUniqAgent::init(int argc, char* argv[])
{
    std::string errmsg;
    if (!utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return false;
    }

    // Check parameters
    if (argument::master->value().empty() && (0 == argument::label->value()))
    {
    	fprintf(stderr, "parameter[master] is empty and parameter[label] is 0.\n");
    	return false;
    }

    try
    {
        sys::g_logger = sys::create_safe_logger();
        if (!restore_sequence())
            return false;
        if (!reselect_master_addr())
            return false;

        _udp_socket = new net::CUdpSocket;
        _udp_socket->listen(argument::ip->value(), argument::port->value());
        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        return false;
    }
}

bool CUniqAgent::run()
{
    while (true)
    {
        int events_returned = 0;
        int events_requested = POLLIN | POLLERR; // POLLIN | POLLOUT | POLLERR;
        int milliseconds = 10000;

        if (!net::CUtils::timed_poll(_udp_socket->get_fd(), events_requested, milliseconds, &events_returned))
        {
            // timeout
        }
        else
        {
            int bytes_received = _udp_socket->receive_from(_request_buffer, sizeof(_request_buffer), &_from_addr);
            if (bytes_received < sizeof(struct MessageHead))
            {
                MYLOG_ERROR("invalid size (%d) from %s: %s\n", bytes_received, net::to_string(_from_addr).c_str(), strerror(errno));
            }
            else
            {
                _message_head = reinterpret_cast<struct MessageHead*>(_request_buffer);
                MYLOG_INFO("%s from %s", _message_head->str().c_str(), net::to_string(_from_addr).c_str());

                if (bytes_received != _message_head->len)
                {
                    MYLOG_ERROR("invalid size (%d/%d) from %s: %s\n", bytes_received, _message_head->len.to_int(), net::to_string(_from_addr).c_str(), strerror(errno));
                }
                else
                {
                    bool prepare_ok = false;

                    if (REQUEST_HOST == _message_head->type)
                    {
                        prepare_response_get_host();
                    }
                    else if (REQUEST_UNIQ_ID == _message_head->type)
                    {
                        prepare_response_get_uniq_id();
                    }
                    else if (REQUEST_UNIQ_SEQ == _message_head->type)
                    {
                        prepare_response_get_uniq_seq();
                    }
                    else if (REQUEST_HOST_AND_SEQ == _message_head->type)
                    {
                        prepare_response_get_host_and_seq();
                    }
                    else
                    {
                        MYLOG_ERROR("invalid message type: %s\n", _message_head->str().c_str());
                    }

                    if (prepare_ok)
                    {
                        try
                        {
                            _udp_socket->send_to(_response_buffer, _response_size, _from_addr);
                            MYLOG_INFO("send to %s ok\n", net::to_string(_from_addr).c_str());
                        }
                        catch (sys::CSyscallException& ex)
                        {
                            MYLOG_ERROR("send to %s failed: %s\n", net::to_string(_from_addr).c_str(), ex.str().c_str());
                        }
                    }
                }
            }
        }
    }
    return true;
}

void CUniqAgent::fini()
{
}

std::string CUniqAgent::get_sequence_path() const
{
    return sys::CUtils::get_program_path() + std::string("/.uniq.seq");
}

uint32_t CUniqAgent::get_label() const
{
	if (argument::master->value().empty())
	{
		return argument::label->value();
	}
	else
	{
		net::CUdpSocket master_socket;
		struct MessageHead head;
		head.len = sizeof(head);
		head.type = REQUEST_LABEL;

		try
		{
			master_socket.send_to(&head, sizeof(head), _master_addr);
		}
		catch (sys::CSyscallException& ex)
		{
			MYLOG_ERROR("%s\n", ex.str().c_str());
		}
		return 0;
	}
}

bool CUniqAgent::reselect_master_addr()
{
    _master_addr.sin_addr.s_addr = inet_addr(argument::master_ip->c_value());
    if (0 == _master_addr.sin_addr.s_addr)
    {
        MYLOG_ERROR("invalid IP: %s\n", argument::master_ip->c_value());
        return false;
    }
    else
    {
        _master_addr.sin_port = htons(argument::master_port->value());
        return true;
    }
}

bool CUniqAgent::restore_sequence()
{
    uint64_t magic = 0;

    int fd = open(_sequence_path.c_str(), O_RDONLY|O_WRONLY|O_CREAT);
    if (-1 == fd)
    {
        MYLOG_ERROR("open %s failed: %s\n", _sequence_path.c_str(), strerror(errno));
        return false;
    }

    sys::CloseHelper ch(fd);
    ssize_t bytes_read = read(fd, &_seq_block, sizeof(_seq_block));
    if (0 == bytes_read)
    {
        MYLOG_INFO("%s empty\n", _sequence_path.c_str());

        _sequence_fd = ch.release();
        _sequence_start = 0;
        _seq_block.label = get_label();
        _seq_block.sequence = SEQUENCE_STEPS;
        return store_sequence();
    }
    else if (bytes_read != sizeof(_seq_block))
    {
        MYLOG_ERROR("read %s failed: (%d/%zd)%s\n", _sequence_path.c_str(), bytes_read, sizeof(_seq_block), strerror(errno));
        return false;
    }
    else
    {
        if (!_seq_block.valid_magic())
        {
            MYLOG_ERROR("%s invalid: %s\n", _seq_block.str().c_str(), _sequence_path.c_str());
            return false;
        }
        else
        {
            _sequence_fd = ch.release();
            _sequence_start = _seq_block.sequence + SEQUENCE_STEPS;
            //_seq_block.label =
            _seq_block.sequence = _sequence_start;
            return store_sequence();
        }
    }
}

bool CUniqAgent::store_sequence()
{
    _seq_block.version = SEQUENCE_BLOCK_VERSION;
    _seq_block.timestamp = time(NULL);
    _seq_block.update_magic();

    ssize_t byes_written = pwrite(_sequence_fd, _seq_block, sizeof(_seq_block), 0);
    if (byes_written != sizeof(_seq_block))
    {
        MYLOG_ERROR("write %s to %s failed: %s\n", _seq_block.str().c_str(), _sequence_path.c_str(), strerror(errno));
        return false;
    }
    else
    {
        MYLOG_INFO("store %s to %s ok\n", _seq_block.str().c_str(), _sequence_path.c_str());
        return true;
    }
}

uint32_t CUniqAgent::inc_sequence()
{
	uint32_t sequence = 0;

	++_seq_block.sequence;
	if (_seq_block.sequence - _sequence_start > SEQUENCE_STEPS)
	{
		if (store_sequence())
			sequence = _seq_block.sequence;
	}

    return sequence;
}

uint64_t CUniqAgent::get_uniq_id(const struct GetUniqIdRequest* request)
{
    struct tm now;
    time_t t = (0 == request->timestamp)? time(NULL): request->timestamp;
    (void)localtime_r(&t, &now);

    union UniqID uniq_id;
    uniq_id.id.user = static_cast<uint8_t>(request->user);
    uniq_id.id.label = static_cast<uint8_t>(_seq_block.label);
    uniq_id.id.year = (now.tm_year+1900)-2016;
    uniq_id.id.month = now.tm_mon+1;
    uniq_id.id.day = now.tm_mday;
    uniq_id.id.hour = now.tm_hour;
    uniq_id.id.seq = inc_sequence();

    return uniq_id.value;
}

bool CUniqAgent::prepare_response_get_label()
{
    struct MessageHead response;

    _response_size = sizeof(struct MessageHead);
    response.len = sizeof(struct MessageHead);
    response.type = RESPONSE_LABEL;
    response.value1 = _seq_block.label;
    return true;
}

bool CUniqAgent::prepare_response_get_uniq_id()
{
    struct GetUniqIdRequest* request = reinterpret_cast<struct GetUniqIdRequest*>(_request_buffer);
    struct MessageHead* response = reinterpret_cast<struct _sequence*>(_response_buffer);

    _response_size = sizeof(struct MessageHead);
    response->len = sizeof(struct MessageHead);
    response->type = RESPONSE_UNIQ_ID;
    response->value1 = get_uniq_id(request);
    response->value2 = 0;
    return true;
}

bool CUniqAgent::prepare_response_get_uniq_seq()
{
    struct GetUniqIdRequest* request = reinterpret_cast<struct GetUniqIdRequest*>(_request_buffer);
    struct MessageHead* response = reinterpret_cast<struct _sequence*>(_response_buffer);

    _response_size = sizeof(struct MessageHead);
    response->len = sizeof(struct MessageHead);
    response->type = RESPONSE_UNIQ_SEQ;
    response->value1 = inc_sequence();
    response->value2 = 0;
    return true;
}

bool CUniqAgent::prepare_response_get_label_and_seq()
{
    struct GetUniqIdRequest* request = reinterpret_cast<struct GetUniqIdRequest*>(_request_buffer);
    struct MessageHead* response = reinterpret_cast<struct _sequence*>(_response_buffer);

    _response_size = sizeof(struct MessageHead);
    response->len = sizeof(struct MessageHead);
    response->type = RESPONSE_LABEL_AND_SEQ;
    response->value1 = _seq_block.sequence;
    response->value2 = _seq_block.label;
    return true;
}

} // namespace mooon {
