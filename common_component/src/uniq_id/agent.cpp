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
#include <mooon/net/epoller.h>
#include <mooon/net/udp_socket.h>
#include <mooon/net/utils.h>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <mooon/utils/tokener.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

// 日志控制：
// 可通过设置环境变量MOOON_LOG_LEVEL和MOOON_LOG_SCREEN来控制日志级别和是否在屏幕上输出日志
// 1) MOOON_LOG_LEVEL可以取值debug,info,error,warn,fatal
// 2) MOOON_LOG_SCREEN取值为1表示在屏幕输出日志，其它则表示不输出

STRING_ARG_DEFINE(master_nodes, "", "master nodes, e.g., 192.168.31.66:2016,192.168.31.88:2016");
STRING_ARG_DEFINE(ip, "0.0.0.0", "listen IP");
INTEGER_ARG_DEFINE(uint16_t, port, 6200, 1000, 65535, "listen port");
INTEGER_ARG_DEFINE(uint8_t, label, 0, 0, LABEL_MAX, "unique label of a machine");
INTEGER_ARG_DEFINE(uint32_t, steps, 100000, 1, 1000000, "steps to store");

// Label过期时长，所有节点的expire值必须保持相同，包括master节点和所有agent节点
// expire值必须大于interval的两倍，且必须大10
INTEGER_ARG_DEFINE(uint32_t, expire, LABEL_EXPIRED_SECONDS, 10, 4294967295U, "label expired seconds");
// 多长间隔向master发一次租赁Lable请求
INTEGER_ARG_DEFINE(uint32_t, interval, 600, 1, 7200, "rent label interval (seconds)");

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

    SeqBlock()
        : version(SEQUENCE_BLOCK_VERSION), label(0), sequence(0), timestamp(0), magic(0)
    {
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string("block://V%u/L%u/S%u/D%s/M%"PRId64, version, label, sequence, sys::CDatetimeUtils::to_datetime(timestamp).c_str(), magic);
    }

    void update_label(uint32_t label_)
    {
        MYLOG_DEBUG("%s => %u\n", str().c_str(), label_);
        label = label_;
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
    int get_label(bool asynchronous);
    bool parse_master_nodes();
    bool restore_sequence();
    bool store_sequence();
    uint32_t inc_sequence();
    uint64_t get_uniq_id(const struct MessageHead* request);
    void rent_label();
    bool label_expired() const;
    bool io_error() const { return _io_error; }
    const struct sockaddr_in& get_master_addr() const;

private:
    void prepare_response_error(int errcode);
    int prepare_response_get_label();
    int prepare_response_get_uniq_id();
    int prepare_response_get_uniq_seq();
    int prepare_response_get_label_and_seq();

private:
    int on_response_error();
    int on_response_label();

private:
    uint32_t _echo;
    std::vector<struct sockaddr_in> _masters_addr;
    net::CEpoller _epoller;
    net::CUdpSocket* _udp_socket;
    uint32_t _sequence_start;
    struct SeqBlock _seq_block;
    std::string _sequence_path;
    int _sequence_fd;
    time_t _current_time; // 当前时间
    time_t _last_rent_time; // 最后一次向master发起rent_label的时间
    bool _io_error; // IO出错标记，将不能继续服务

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
    : _echo(0), _udp_socket(NULL), _sequence_fd(-1), _current_time(0), _last_rent_time(0), _io_error(false),
      _old_seq(0), _old_hour(-1), _old_day(-1), _old_month(-1), _old_year(-1),
      _message_head(NULL)
{
    _sequence_start = 0;
    _sequence_path = get_sequence_path();

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
    if (argument::master_nodes->value().empty() && (0 == argument::label->value()))
    {
    	fprintf(stderr, "parameter[master] is empty and parameter[label] is 0 at the same time.\n");
    	return false;
    }
    if ((argument::expire->value() < argument::interval->value() * 2) ||
        (argument::expire->value() < argument::interval->value() + 10))
    {
        fprintf(stderr, "parameter[expire] should greater than interval with 10 and double\n");
        return false;
    }

    if (!parse_master_nodes())
    {
        return false;
    }

    try
    {
        sys::g_logger = sys::create_safe_logger();
        _current_time = time(NULL);

        _epoller.create(10);
        _udp_socket = new net::CUdpSocket;
        _udp_socket->listen(argument::ip->value(), argument::port->value(), true);
        MYLOG_INFO("listen on %s:%d\n", argument::ip->c_value(), argument::port->value());
        _epoller.set_events(_udp_socket, EPOLLIN);

        if (!restore_sequence())
            return false;
        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        if (sys::g_logger != NULL)
        {
            MYLOG_ERROR("%s\n", ex.str().c_str());
        }
        return false;
    }
}

bool CUniqAgent::run()
{
    while (true)
    {
        const int milliseconds = 10000;
        int n = _epoller.timed_wait(milliseconds);

        // 不需要那么精确的时间
        _current_time = time(NULL);
        if (_current_time - _last_rent_time > static_cast<time_t>(argument::interval->value()))
        {
            // 间隔的向master发一个续租请求
            rent_label();
            _last_rent_time = _current_time;
        }

        if (0 == n)
        {
            // timeout, do nothing
        }
        else
        {
            // 循环，可以减少对CEpoller::timed_wait的调用
            for (int i=0; i<10000; ++i)
            {
                try
                {
                    // recvmmsg
                    int bytes_received = _udp_socket->receive_from(_request_buffer, sizeof(_request_buffer), &_from_addr);
                    if (-1 == bytes_received)
                    {
                        // WOULDBLOCK
                        break;
                    }
                    else if (bytes_received < static_cast<int>(sizeof(struct MessageHead)))
                    {
                        MYLOG_ERROR("invalid size (%d) from %s: %s\n", bytes_received, net::to_string(_from_addr).c_str(), strerror(errno));
                    }
                    else
                    {
                        _message_head = reinterpret_cast<struct MessageHead*>(_request_buffer);
                        MYLOG_DEBUG("%s from %s", _message_head->str().c_str(), net::to_string(_from_addr).c_str());

                        if (bytes_received != _message_head->len)
                        {
                            MYLOG_ERROR("invalid size (%d/%d) from %s: %s\n", bytes_received, _message_head->len.to_int(), net::to_string(_from_addr).c_str(), strerror(errno));
                        }
                        else
                        {
                            int errcode = 0;
                            std::string errmsg;

                            // Request from client
                            if (REQUEST_LABEL == _message_head->type)
                            {
                                errcode = prepare_response_get_label();
                            }
                            else if (REQUEST_UNIQ_ID == _message_head->type)
                            {
                                errcode = prepare_response_get_uniq_id();
                            }
                            else if (REQUEST_UNIQ_SEQ == _message_head->type)
                            {
                                errcode = prepare_response_get_uniq_seq();
                            }
                            else if (REQUEST_LABEL_AND_SEQ == _message_head->type)
                            {
                                errcode = prepare_response_get_label_and_seq();
                            }
                            // Response from master
                            else if (RESPONSE_ERROR == _message_head->type)
                            {
                                errcode = on_response_error();
                            }
                            else if (RESPONSE_LABEL == _message_head->type)
                            {
                                errcode = on_response_label();
                            }
                            else
                            {
                                errcode = ERROR_INVALID_TYPE;
                                MYLOG_ERROR("invalid message type: %s\n", _message_head->str().c_str());
                            }
                            if ((errcode != 0) && (errcode != -1))
                            {
                                prepare_response_error(errcode);
                            }

                            if (errcode != -1)
                            {
                                try
                                {
                                    _udp_socket->send_to(_response_buffer, _response_size, _from_addr);
                                    MYLOG_DEBUG("send to %s ok\n", net::to_string(_from_addr).c_str());
                                }
                                catch (sys::CSyscallException& ex)
                                {
                                    MYLOG_ERROR("send to %s failed: %s\n", net::to_string(_from_addr).c_str(), ex.str().c_str());
                                }
                            }
                        }
                    }
                }
                catch (sys::CSyscallException& ex)
                {
                    MYLOG_ERROR("receive_from failed: %s\n", ex.str().c_str());
                    break;
                }
            } // while (true)
        } // if (0 == n)
    } // while (true)

    return true;
}

void CUniqAgent::fini()
{
}

std::string CUniqAgent::get_sequence_path() const
{
    return sys::CUtils::get_program_path() + std::string("/.uniq.seq");
}

int CUniqAgent::get_label(bool asynchronous)
{
	if (argument::master_nodes->value().empty())
	{
		return argument::label->value();
	}
	else
	{
		struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
		struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);
		const struct sockaddr_in& master_addr = get_master_addr();

        // 遇到错误ERROR_LABEL_NOT_HOLD时，需要重试一次
        for (int k=0; k<2; ++k)
        {
            try
            {
                request->len = sizeof(struct MessageHead);
                request->type = REQUEST_LABEL;
                request->echo = _echo++;
                request->value1 = _seq_block.label;
                request->value2 = 0;
                _udp_socket->send_to(_request_buffer, sizeof(struct MessageHead), master_addr);

                if (asynchronous)
                {
                    return 0;
                }
                else
                {
                    int bytes = _udp_socket->timed_receive_from(_response_buffer, sizeof(struct MessageHead), &_from_addr, 2000);
                    if (bytes != sizeof(struct MessageHead))
                    {
                        MYLOG_ERROR("timed_receive_from return %d(%d)\n", bytes, static_cast<int>(sizeof(struct MessageHead)));
                        break;
                    }

                    if (RESPONSE_ERROR == response->type)
                    {
                        MYLOG_ERROR("(%d)get label[%u] error: %s\n", k, _seq_block.label, response->str().c_str());
                        if (response->value1.to_int() != ERROR_LABEL_NOT_HOLD)
                            break;

                        // 需要重新租赁Label，故重置
                        _seq_block.update_label(0);
                        continue;
                    }
                    else if ((RESPONSE_LABEL == response->type) && (response->echo == _echo-1))
                    {
                        if (response->value1.to_int() > 0)
                        {
                            // 续成功
                            _seq_block.timestamp = static_cast<uint64_t>(_current_time);
                            int label = static_cast<int>(response->value1.to_int());
                            MYLOG_INFO("rent label[%d] ok\n", label);
                            return label;
                        }
                        else
                        {
                            MYLOG_ERROR("invalid label[%d] from %s\n", (int)response->value1.to_int(), net::to_string(_from_addr).c_str());
                            break;
                        }
                    }
                    else
                    {
                        MYLOG_ERROR("invalid response[%s] for request[%s] from %s\n", response->str().c_str(), request->str().c_str(), net::to_string(_from_addr).c_str());
                        break;
                    }
                }
            }
            catch (sys::CSyscallException& ex)
            {
                MYLOG_ERROR("rent label from %s faield: %s\n", net::to_string(_from_addr).c_str(), ex.str().c_str());
                break;
            }
        } // for

		return -1;
	}
}

bool CUniqAgent::parse_master_nodes()
{
    const std::string& master_nodes = argument::master_nodes->value();
    utils::CEnhancedTokener tokener;

    tokener.parse(master_nodes, ",", ':');
    const std::map<std::string, std::string>& tokens = tokener.tokens();
    for (std::map<std::string, std::string>::const_iterator iter=tokens.begin(); iter!=tokens.end(); ++iter)
    {
        const std::string& ip_str = iter->first;
        const std::string& port_str = iter->second;

        uint32_t ip = net::string2ipv4(ip_str);
        if (0 == ip)
        {
            fprintf(stderr, "parameter[master_nodes] error: %s\n", master_nodes.c_str());
            return false;
        }

        int port = atoi(port_str.c_str());
        if ((port < 1000) || (port > 65535))
        {
            fprintf(stderr, "parameter[master_nodes] error: %s\n", master_nodes.c_str());
            return false;
        }

        struct sockaddr_in master_addr;
        master_addr.sin_family = AF_INET;
        master_addr.sin_addr.s_addr = ip;
        master_addr.sin_port = net::CUtils::host2net(static_cast<uint16_t>(port));
        memset(master_addr.sin_zero, 0, sizeof(master_addr.sin_zero));
        _masters_addr.push_back(master_addr);
    }

    return true;
}

bool CUniqAgent::restore_sequence()
{
    int label = 0;

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

        label = get_label(false);
        if ((label < 1) || (label > LABEL_MAX))
        {
            MYLOG_ERROR("invalid label[%d]\n", label);
            return false;
        }

        _sequence_fd = ch.release();
        _sequence_start = argument::steps->value();
        _seq_block.sequence = _sequence_start;
        _seq_block.update_label(static_cast<uint32_t>(label));
        return store_sequence();
    }
    else if (-1 == bytes_read)
    {
        // IO error
        MYLOG_ERROR("read %s failed: %s\n", _sequence_path.c_str(), strerror(errno));
        return false;
    }
    else if (bytes_read != sizeof(_seq_block))
    {
        // invalid block
        MYLOG_ERROR("read %s failed: (%d/%zd)%s\n", _sequence_path.c_str(), bytes_read, sizeof(_seq_block), strerror(errno));
        return false;
    }
    else
    {
        // check block
        if (!_seq_block.valid_magic())
        {
            MYLOG_ERROR("%s invalid: %s\n", _seq_block.str().c_str(), _sequence_path.c_str());
            return false;
        }
        else
        {
            if (argument::master_nodes->value().empty())
            {
                // 本地模式
                label = argument::label->value();
            }
            else if (label_expired())
            {
                // 如果已过期，则需要重新租赁一个
                label = get_label(false);
                if ((label < 1) || (label > LABEL_MAX))
                {
                    MYLOG_ERROR("invalid label[%d] from master to store\n", label);
                    return false;
                }
            }

            _sequence_fd = ch.release();
            // 多加一次steps，原因是store时未调用fsync
            _sequence_start = _seq_block.sequence + (2 * argument::steps->value());
            _seq_block.sequence = _sequence_start;
            _seq_block.update_label(static_cast<uint32_t>(label));

            return store_sequence();
        }
    }
}

bool CUniqAgent::store_sequence()
{
    _seq_block.update_magic();

    ssize_t byes_written = pwrite(_sequence_fd, &_seq_block, sizeof(_seq_block), 0);
    if (byes_written != sizeof(_seq_block))
    {
        _io_error = true; // 遇到IO错误时，标记为不可继续服务
        MYLOG_ERROR("store %s to %s failed: %s\n", _seq_block.str().c_str(), _sequence_path.c_str(), strerror(errno));
        return false;
    }
    else
    {
        MYLOG_DEBUG("store %s ok\n", _seq_block.str().c_str());
#if 1
        return true;
#else
        // fsync严重影响性能
        if (-1 == fsync(_sequence_fd))
        {
            _io_error = true;
            MYLOG_ERROR("fsync %s to %s failed: %s\n", _seq_block.str().c_str(), _sequence_path.c_str(), strerror(errno));
            return false;
        }
        else
        {
            _sequence_start = _seq_block.sequence;
            MYLOG_INFO("store %s to %s ok\n", _seq_block.str().c_str(), _sequence_path.c_str());
            return true;
        }
#endif
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
    	static struct tm old_tm;
    	static time_t old_time = 0;

    	struct tm* now = &old_tm;
        time_t current_time = static_cast<time_t>(request->value2.to_int());
        if (0 == current_time)
        {
        	current_time = _current_time;
        }
        if (current_time - old_time > 30) // current_time != old_time
        {
        	// 由于只取小时，因此理论上每小时调用一次localtime即可
        	now = localtime(&current_time); // localtime和localtime_r开销较大
        	old_tm = *now; // Rember
        	old_time = current_time; // Rember
        }

        union UniqID uniq_id;
        uniq_id.id.user = static_cast<uint8_t>(request->value1.to_int());
        uniq_id.id.label = static_cast<uint8_t>(_seq_block.label);
        uniq_id.id.year = (now->tm_year+1900) - BASE_YEAR;
        uniq_id.id.month = now->tm_mon+1;
        uniq_id.id.day = now->tm_mday;
        uniq_id.id.hour = now->tm_hour;
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
            _old_seq = seq;
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
    if (!argument::master_nodes->value().empty())
    {
        int label = get_label(true);

        if (label > 0)
        {
            _seq_block.update_label(label);
        }
    }
}

bool CUniqAgent::label_expired() const
{
    if (argument::master_nodes->value().empty())
        return false;

    bool expired = _current_time - static_cast<time_t>(_seq_block.timestamp) > static_cast<time_t>(argument::expire->value());
    if (expired)
    {
        MYLOG_ERROR("Label[%u] expired(%u): %s\n", _seq_block.label, argument::expire->value(), sys::CDatetimeUtils::to_datetime(static_cast<time_t>(_seq_block.timestamp)).c_str());
    }
    return expired;
}

// 轮询方式
const struct sockaddr_in& CUniqAgent::get_master_addr() const
{
    static uint32_t i = 0;
    return _masters_addr[i++ % _masters_addr.size()];
}

void CUniqAgent::prepare_response_error(int errcode)
{
    struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

    _response_size = sizeof(struct MessageHead);
    response->len = sizeof(struct MessageHead);
    response->type = RESPONSE_ERROR;
    response->echo = request->echo;
    response->value1 = errcode;
    response->value2 = 0;

    MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
}

int CUniqAgent::prepare_response_get_label()
{
    if (label_expired())
    {
        return ERROR_LABEL_EXPIRED;
    }
    else if (io_error())
    {
        return ERROR_STORE_SEQ;
    }
    else
    {
        struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
        struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

        _response_size = sizeof(struct MessageHead);
        response->len = sizeof(struct MessageHead);
        response->type = RESPONSE_LABEL;
        response->echo = request->echo;
        response->value1 = _seq_block.label;
        response->value2 = 0;

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
    else if (io_error())
    {
        return ERROR_STORE_SEQ;
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
            response->echo = request->echo;
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
    else if (io_error())
    {
        return ERROR_STORE_SEQ;
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
            struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
            struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

            _response_size = sizeof(struct MessageHead);
            response->len = sizeof(struct MessageHead);
            response->type = RESPONSE_UNIQ_SEQ;
            response->echo = request->echo;
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
    else if (io_error())
    {
        return ERROR_STORE_SEQ;
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
            struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
            struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

            _response_size = sizeof(struct MessageHead);
            response->len = sizeof(struct MessageHead);
            response->type = RESPONSE_LABEL_AND_SEQ;
            response->echo = request->echo;
            response->value1 = _seq_block.label;
            response->value2 = seq;

            MYLOG_DEBUG("prepare %s ok\n", response->str().c_str());
            return 0;
        }
    }
}

int CUniqAgent::on_response_error()
{
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_request_buffer);
    MYLOG_ERROR("%s from %s\n", response->str().c_str(), net::to_string(_from_addr).c_str());

    if (ERROR_LABEL_NOT_HOLD == response->value1.to_int())
    {
        // 需要重新租赁Label，故重置
        _seq_block.update_label(0);
        get_label(true);
    }

    return -1;
}

int CUniqAgent::on_response_label()
{
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_request_buffer);
    MYLOG_INFO("%s from %s\n", response->str().c_str(), net::to_string(_from_addr).c_str());

    uint32_t old_label = _seq_block.label;
    _seq_block.update_label(static_cast<uint32_t>(response->value1.to_int()));
    _seq_block.timestamp = static_cast<uint64_t>(_current_time);

    // Lable发生变化时，立即保存
    if (old_label != _seq_block.label)
        (void)store_sequence();
    return -1;
}

} // namespace mooon {
