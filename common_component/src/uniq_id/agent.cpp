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
#include <fcntl.h>
#include <mooon/net/udp_socket.h>
#include <mooon/net/utils.h>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/datetime_utils.h>
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
INTEGER_ARG_DEFINE(uint16_t, port, 6200, 1000, 65535, "listen port");
INTEGER_ARG_DEFINE(uint8_t, label, 0, 0, LABEL_MAX, "unique label of a machine");
INTEGER_ARG_DEFINE(uint16_t, steps, 1000, 1, 65535, "steps to store");
INTEGER_ARG_DEFINE(uint32_t, expire, LABEL_EXPIRED_SECONDS, 1, LABEL_EXPIRED_SECONDS*10, "label expired seconds");

////////////////////////////////////////////////////////////////////////////////
namespace mooon {

// 常量
enum
{
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
        return utils::CStringUtils::format_string("block://%u/%u/%u/%s/%"PRId64, version, label, sequence, sys::CDatetimeUtils::to_datetime(timestamp).c_str(), magic);
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
    uint8_t get_label();
    bool restore_sequence();
    bool store_sequence();
    uint32_t inc_sequence();
    uint64_t get_uniq_id(const struct MessageHead* request);
    void rent_label();
    bool label_expired() const;

private:
    void prepare_response_error(int errcode);
    int prepare_response_get_label();
    int prepare_response_get_uniq_id();
    int prepare_response_get_uniq_seq();
    int prepare_response_get_label_and_seq();

private:
    uint32_t _echo;
    std::vector<struct sockaddr_in> _masters_addr;
    net::CUdpSocket* _udp_socket;
    uint32_t _sequence_start;
    struct SeqBlock _seq_block;
    std::string _sequence_path;
    int _sequence_fd;
    time_t _current_time; // 当前时间
    time_t _last_rent_time; // 最近一次续租Lable的时间

private:
    // old系列变量用来解决seq用完问题，
    // 一个小时内全部用完，则不能再提供服务，因为会导致重复的ID
    uint32_t _old_seq;
    int _old_hour;
    int _old_day;
    int _old_month;
    int _old_year;

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
    : _echo(0), _udp_socket(NULL), _sequence_fd(-1), _current_time(0), _last_rent_time(0),
      _old_seq(0), _old_hour(-1), _old_day(-1), _old_month(-1), _old_year(-1),
      _message_head(NULL)
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
    	fprintf(stderr, "parameter[master] is empty and parameter[label] is 0 at the same time.\n");
    	return false;
    }

    try
    {
        sys::g_logger = sys::create_safe_logger();
        _current_time = time(NULL);
        if (!restore_sequence())
            return false;

        _udp_socket = new net::CUdpSocket;
        _udp_socket->listen(argument::ip->value(), argument::port->value());
        MYLOG_INFO("listen on %s:%d\n", argument::ip->c_value(), argument::port->value());
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
    time_t old_time = time(NULL);

    while (true)
    {
        int events_returned = 0;
        int events_requested = POLLIN | POLLERR; // POLLIN | POLLOUT | POLLERR;
        int milliseconds = 10000;

        if (!net::CUtils::timed_poll(_udp_socket->get_fd(), events_requested, milliseconds, &events_returned))
        {
            _current_time = time(NULL);
            if (_current_time - old_time > 600)
            {
                // 续租
                rent_label();
            }
        }
        else
        {
            int bytes_received = _udp_socket->receive_from(_request_buffer, sizeof(_request_buffer), &_from_addr);
            if (bytes_received < static_cast<int>(sizeof(struct MessageHead)))
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
                    int result = 0;
                    std::string errmsg;
                    _last_rent_time = time(NULL);

                    if (REQUEST_LABEL == _message_head->type)
                    {
                        result = prepare_response_get_label();
                    }
                    else if (REQUEST_UNIQ_ID == _message_head->type)
                    {
                        result = prepare_response_get_uniq_id();
                    }
                    else if (REQUEST_UNIQ_SEQ == _message_head->type)
                    {
                        result = prepare_response_get_uniq_seq();
                    }
                    else if (REQUEST_LABEL_AND_SEQ == _message_head->type)
                    {
                        result = prepare_response_get_label_and_seq();
                    }
                    else
                    {
                        result = ERROR_INVALID_TYPE;
                        MYLOG_ERROR("invalid message type: %s\n", _message_head->str().c_str());
                    }
                    if (result != 0)
                    {
                        prepare_response_error(result);
                    }

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
    return true;
}

void CUniqAgent::fini()
{
}

std::string CUniqAgent::get_sequence_path() const
{
    return sys::CUtils::get_program_path() + std::string("/.uniq.seq");
}

uint8_t CUniqAgent::get_label()
{
	if (argument::master->value().empty())
	{
		return argument::label->value();
	}
	else
	{
		net::CUdpSocket master_socket;
		struct sockaddr_in master_addr;
		struct MessageHead response;
		struct MessageHead request;
		request.len = sizeof(request);
		request.type = REQUEST_LABEL;
		request.echo = _echo++;
		request.value1 = _seq_block.label;
		request.value2 = 0;

		for (std::vector<struct sockaddr_in>::size_type i=0; i<_masters_addr.size(); ++i)
		{
            try
            {
                master_socket.send_to(reinterpret_cast<char*>(&request), sizeof(request), _masters_addr[i]);
                master_socket.receive_from(&response, sizeof(response), &master_addr);

                if ((RESPONSE_LABEL == response.type) && (response.echo == _echo-1))
                {
                    if (response.value1.to_int() > 0)
                    {
                        // 续成功
                        _last_rent_time = _current_time;
                        return static_cast<uint8_t>(response.value1.to_int());
                    }
                    else
                    {
                        MYLOG_ERROR("invalid label[%d] from %s\n", (int)response.value1.to_int(), net::to_string(_masters_addr[i]).c_str());
                    }
                }
                else
                {
                    MYLOG_ERROR("invalid response[%s] for request[%s] from %s\n", response.str().c_str(), request.str().c_str(), net::to_string(_masters_addr[i]).c_str());
                }
            }
            catch (sys::CSyscallException& ex)
            {
                MYLOG_ERROR("rent lable from %s faield: %s\n", net::to_string(_masters_addr[i]).c_str(), ex.str().c_str());
            }
		}

		return 0;
	}
}

bool CUniqAgent::restore_sequence()
{
    uint8_t label = get_label();
    if (0 == label)
        return false;

    int fd = open(_sequence_path.c_str(), O_RDWR|O_CREAT, FILE_DEFAULT_PERM);
    if (-1 == fd)
    {
        MYLOG_ERROR("open %s failed: %s\n", _sequence_path.c_str(), strerror(errno));
        return false;
    }

    sys::CloseHelper<int> ch(fd);
    ssize_t bytes_read = pread(fd, &_seq_block, sizeof(_seq_block), 0);
    if (0 == bytes_read)
    {
        MYLOG_INFO("%s empty\n", _sequence_path.c_str());

        _sequence_fd = ch.release();
        _sequence_start = argument::steps->value();
        _seq_block.label = label;
        _seq_block.sequence = _sequence_start;
        return store_sequence();
    }
    else if (-1 == bytes_read)
    {
        MYLOG_ERROR("read %s failed: %s\n", _sequence_path.c_str(), strerror(errno));
        return false;
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
            _sequence_start = _seq_block.sequence + argument::steps->value();
            _seq_block.label = label;
            _seq_block.sequence = _sequence_start;
            return store_sequence();
        }
    }
}

bool CUniqAgent::store_sequence()
{
    MYLOG_INFO("%s\n", _seq_block.str().c_str());

    _seq_block.version = SEQUENCE_BLOCK_VERSION;
    _seq_block.timestamp = _current_time;
    _seq_block.update_magic();

    ssize_t byes_written = pwrite(_sequence_fd, &_seq_block, sizeof(_seq_block), 0);
    if (byes_written != sizeof(_seq_block))
    {
        MYLOG_ERROR("write %s to %s failed: %s\n", _seq_block.str().c_str(), _sequence_path.c_str(), strerror(errno));
        return false;
    }
    else
    {
        if (-1 == fsync(_sequence_fd))
        {
            MYLOG_ERROR("fsync %s to %s failed: %s\n", _seq_block.str().c_str(), _sequence_path.c_str(), strerror(errno));
            return false;
        }
        else
        {
            _sequence_start = _seq_block.sequence;
            MYLOG_INFO("store %s to %s ok\n", _seq_block.str().c_str(), _sequence_path.c_str());
            return true;
        }
    }
}

uint32_t CUniqAgent::inc_sequence()
{
    bool stored = true;
	uint32_t sequence = 0;

	if ((_seq_block.sequence < _sequence_start) ||
	    (_seq_block.sequence - _sequence_start > argument::steps->value()))
	{
	    stored = store_sequence();
	}
	if (stored)
	{
	    while (0 == sequence)
	        sequence = _seq_block.sequence++;
	}

    return sequence;
}

uint64_t CUniqAgent::get_uniq_id(const struct MessageHead* request)
{
    uint32_t seq = inc_sequence();

    if (0 == seq)
    {
        return 0; // store sequence block failed
    }
    else
    {
        struct tm now;
        time_t t = static_cast<time_t>(request->value2.to_int());
        if (0 == t)
            t = _current_time;
        (void)localtime_r(&t, &now);

        union UniqID uniq_id;
        uniq_id.id.user = static_cast<uint8_t>(request->value1.to_int());
        uniq_id.id.label = static_cast<uint8_t>(_seq_block.label);
        uniq_id.id.year = (now.tm_year+1900) - BASE_YEAR;
        uniq_id.id.month = now.tm_mon+1;
        uniq_id.id.day = now.tm_mday;
        uniq_id.id.hour = now.tm_hour;
        uniq_id.id.seq = seq;

        if ((_old_seq > seq) &&
            (_old_hour == static_cast<int>(uniq_id.id.hour)) &&
            (_old_day == static_cast<int>(uniq_id.id.day)) &&
            (_old_month == static_cast<int>(uniq_id.id.month)) &&
            (_old_year == static_cast<int>(uniq_id.id.year)))
        {
            MYLOG_ERROR("sequence overflow\n");
            return 1; // overflow
        }
        else
        {
            _old_hour = uniq_id.id.hour;
            _old_day = uniq_id.id.day;
            _old_month = uniq_id.id.month;
            _old_year = uniq_id.id.year;
            return uniq_id.value;
        }
    }
}

void CUniqAgent::rent_label()
{
    if (!argument::master->value().empty())
    {
        uint8_t label = get_label();

        if (label > 0)
        {
            _seq_block.label = label;
        }
    }
}

bool CUniqAgent::label_expired() const
{
    if (argument::master->value().empty())
        return false;

    return _current_time - _last_rent_time > static_cast<time_t>(argument::expire->value());
}

void CUniqAgent::prepare_response_error(int errcode)
{
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

    _response_size = sizeof(struct MessageHead);
    response->len = sizeof(struct MessageHead);
    response->type = RESPONSE_ERROR;
    response->value1 = errcode;

    MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
}

int CUniqAgent::prepare_response_get_label()
{
    if (label_expired())
    {
        return ERROR_LABEL_EXPIRED;
    }
    else
    {
        struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

        _response_size = sizeof(struct MessageHead);
        response->len = sizeof(struct MessageHead);
        response->type = RESPONSE_LABEL;
        response->value1 = _seq_block.label;

        MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
        return 0;
    }
}

int CUniqAgent::prepare_response_get_uniq_id()
{
    if (label_expired())
    {
        return ERROR_LABEL_EXPIRED;
    }
    else
    {
        struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
        struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

        uint64_t uniq_id = get_uniq_id(request);
        if (0 == uniq_id)
        {
            return ERROR_STORE_SEQ;
        }
        else if (1 == uniq_id)
        {
            return ERROR_OVERFLOW;
        }
        else
        {
            _response_size = sizeof(struct MessageHead);
            response->len = sizeof(struct MessageHead);
            response->type = RESPONSE_UNIQ_ID;
            response->value1 = uniq_id;
            response->value2 = 0;

            MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
            return 0;
        }
    }
}

int CUniqAgent::prepare_response_get_uniq_seq()
{
    if (label_expired())
    {
        return ERROR_LABEL_EXPIRED;
    }
    else
    {
        uint32_t seq = inc_sequence();

        if (0 == seq)
        {
            return ERROR_STORE_SEQ;
        }
        else
        {
            struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

            _response_size = sizeof(struct MessageHead);
            response->len = sizeof(struct MessageHead);
            response->type = RESPONSE_UNIQ_SEQ;
            response->value1 = inc_sequence();
            response->value2 = 0;

            MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
            return 0;
        }
    }
}

int CUniqAgent::prepare_response_get_label_and_seq()
{
    if (label_expired())
    {
        return ERROR_LABEL_EXPIRED;
    }
    else
    {
        uint32_t seq = inc_sequence();

        if (0 == seq)
        {
            return ERROR_STORE_SEQ;
        }
        else
        {
            struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

            _response_size = sizeof(struct MessageHead);
            response->len = sizeof(struct MessageHead);
            response->type = RESPONSE_LABEL_AND_SEQ;
            response->value1 = seq;
            response->value2 = _seq_block.label;

            MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
            return 0;
        }
    }
}

} // namespace mooon {
