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
#include <map>
#include <mooon/net/udp_socket.h>
#include <mooon/net/utils.h>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

STRING_ARG_DEFINE(ip, "0.0.0.0", "listen IP");
INTEGER_ARG_DEFINE(uint16_t, port, 16200, 1000, 65535, "listen port");

STRING_ARG_DEFINE(db_host, "", "MySQL host to connect");
INTEGER_ARG_DEFINE(uint16_t, db_port, 3306, 1000, 65535, "MySQL port to connect");
STRING_ARG_DEFINE(db_user, "root", "MySQL user");
STRING_ARG_DEFINE(db_pass, "", "MySQL password");
STRING_ARG_DEFINE(db_name, "", "MySQL database name");

INTEGER_ARG_DEFINE(uint32_t, expire, LABEL_EXPIRED_SECONDS, 1, LABEL_EXPIRED_SECONDS*10, "label expired seconds");
INTEGER_ARG_DEFINE(uint32_t, timeout, 3600, 1, 36000, "timeout seconds");

////////////////////////////////////////////////////////////////////////////////
namespace mooon {

/*
 * Label资源池表
DROP TABLE IF EXISTS t_label_pool;
CREATE TABLE t_label_pool (
    f_label TINYINT UNSIGNED NOT NULL,
    PRIMARY KEY (f_label)
);
*/

/*
 * Label在线表
DROP TABLE IF EXISTS t_label_online;
CREATE TABLE t_label_online (
    f_ip CHAR(16) NOT NULL,
    f_label TINYINT UNSIGNED NOT NULL,
    f_time DATETIME NOT NULL,
    PRIMARY KEY (f_ip),
    UNIQUE KEY idx_label(f_label),
    KEY idx_time(f_time)
);
*/

/*
 * Label状态表
DROP TABLE IF EXISTS t_label_state;
CREATE TABLE t_label_state (
    f_ip CHAR(16) NOT NULL,
    f_label TINYINT UNSIGNED NOT NULL,
    f_first_time DATETIME NOT NULL,
    f_last_time DATETIME NOT NULL,
    f_state TINYINT NOT NULL,
    PRIMARY KEY (f_ip),
    KEY idx_label(f_label),
    KEY idx_state(f_state),
    KEY idx_first_time(f_first_time),
    KEY idx_last_time(f_last_time)
);
*/

// 标签
struct LabelInfo
{
    uint8_t label;     // Label值
    uint32_t ip;       // 租用它的IP
    time_t lease_time; // 最近一次续租时间

    LabelInfo()
        : label(0), lease_time(0)
    {
    }

    LabelInfo(uint8_t label_, uint32_t ip_, time_t lease_time_=0)
        : label(label_), ip(ip_), lease_time(lease_time_)
    {
    }

    LabelInfo(uint8_t label_, const std::string& ip_, time_t lease_time_=0)
        : label(label_), ip(inet_addr(ip_.c_str())), lease_time(lease_time_)
    {
    }

    std::string str() const
    {
        struct in_addr in;
        in.s_addr = ip;
        return mooon::utils::CStringUtils::format_string("label://%d/%s/%s", (int)label, inet_ntoa(in), sys::CDatetimeUtils::to_datetime(lease_time).c_str());
    }
};

class CUniqMaster: public sys::IMainHelper
{
public:
    CUniqMaster();
    ~CUniqMaster();

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();

private:
    bool check_parameter();
    bool init_mysql();
    bool generate_labels();
    bool load_labels();
    int alloc_label();
    int get_label() const;
    bool hold_label(uint8_t label) const;

private:
    void on_timeout();
    void prepare_response_error(int errcode);
    int prepare_response_get_label();

private:
    time_t _current_time;
    net::CUdpSocket* _udp_socket;
    sys::CMySQLConnection* _mysql;
    std::map<uint8_t, struct LabelInfo> _label_info_map; // 申请续租的

private:
    struct sockaddr_in _from_addr;
    const struct MessageHead* _message_head;
    char _request_buffer[SOCKET_BUFFER_SIZE];
    char _response_buffer[SOCKET_BUFFER_SIZE];
    size_t _response_size;
};

extern "C" int main(int argc, char* argv[])
{
    CUniqMaster master;
    return sys::main_template(&master, argc, argv);
}

CUniqMaster::CUniqMaster()
    : _current_time(0), _udp_socket(NULL), _mysql(NULL), _message_head(NULL)
{
    memset(&_from_addr, 0, sizeof(_from_addr));
    memset(&_request_buffer, 0, sizeof(_request_buffer));
    memset(&_response_buffer, 0, sizeof(_response_buffer));
    _response_size = 0;
}

CUniqMaster::~CUniqMaster()
{
    delete _udp_socket;
    delete _mysql;
}

bool CUniqMaster::init(int argc, char* argv[])
{
    std::string errmsg;
    if (!utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return false;
    }
    if (!check_parameter())
    {
        return false;
    }

    try
    {
        sys::g_logger = sys::create_safe_logger();
        if (!init_mysql())
            return false;
        if (!generate_labels())
            return false;

        _udp_socket = new net::CUdpSocket;
        _udp_socket->listen(argument::ip->value(), argument::port->value());
        MYLOG_INFO("listen on %s:%d\n", argument::ip->c_value(), argument::port->value());
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

bool CUniqMaster::run()
{
    time_t old_time = time(NULL);

    while (true)
    {
        int events_returned = 0;
        int events_requested = POLLIN | POLLERR; // POLLIN | POLLOUT | POLLERR;
        int milliseconds = 10000; // 10seconds

        bool not_timeout = net::CUtils::timed_poll(_udp_socket->get_fd(), events_requested, milliseconds, &events_returned);
        _current_time = time(NULL);
        if (!not_timeout)
        {
            if (_current_time - old_time > static_cast<time_t>(argument::timeout->value()))
                on_timeout();
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
                MYLOG_DEBUG("%s from %s", _message_head->str().c_str(), net::to_string(_from_addr).c_str());

                if (bytes_received != _message_head->len)
                {
                    MYLOG_ERROR("invalid size (%d/%d) from %s: %s\n", bytes_received, _message_head->len.to_int(), net::to_string(_from_addr).c_str(), strerror(errno));
                }
                else
                {
                    int errocode = 0;

                    if (REQUEST_LABEL == _message_head->type)
                    {
                        errocode = prepare_response_get_label();
                    }
                    else
                    {
                        errocode = ERROR_INVALID_TYPE;
                        MYLOG_ERROR("invalid message type: %s\n", _message_head->str().c_str());
                    }
                    if (errocode != 0)
                    {
                        prepare_response_error(errocode);
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

void CUniqMaster::fini()
{
}

bool CUniqMaster::check_parameter()
{
    return true;
}

bool CUniqMaster::init_mysql()
{
    _mysql = new sys::CMySQLConnection;
    _mysql->set_host(argument::db_host->value(), argument::db_port->value());
    _mysql->set_db_name(argument::db_name->value());
    _mysql->set_user(argument::db_user->value(), argument::db_pass->value());
    //_mysql->set_charset();
    _mysql->enable_auto_reconnect();

    try
    {
        _mysql->open();
        MYLOG_INFO("connect %s ok\n", _mysql->str().c_str());
        _mysql->enable_autocommit(false);
        return true;
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("connect %s failed: %s\n", _mysql->str().c_str(), ex.str().c_str());
        return false;
    }
}

bool CUniqMaster::generate_labels()
{
    try
    {
        std::string str = _mysql->query("SELECT count(1) FROM t_label_pool");
        if (str == "0")
        {
            for (int label=0; label<=LABEL_MAX; ++label)
                _mysql->update("INSERT INTO t_label_pool (f_label) VALUES (%d)", label);
            _mysql->commit();
            MYLOG_INFO("generate labels ok\n");
        }

        return true;
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("generate labels failed: %s\n", ex.str().c_str());

        try
        {
            _mysql->rollback();
        }
        catch (sys::CDBException& ex)
        {
            MYLOG_ERROR("rollback failed: %s\n", ex.str().c_str());
        }

        return false;
    }
}

int CUniqMaster::alloc_label()
{
    try
    {
        int label = 0;

        while (true)
        {
            const std::string label_str = _mysql->query("SELECT f_label FROM t_label_pool WHERE f_label<=%d AND f_label>0 LIMIT 1", LABEL_MAX);
            if (label_str.empty())
                break;

            // 被其它master抢走时，返回值为0
            int n = _mysql->update("DELETE FROM t_label_pool WHERE f_label=%s", label_str.c_str());
            if (1 == n)
            {
                n = _mysql->update("INSERT INTO t_label_online (f_ip,f_label,f_time) VALUES (\"%s\",%s,\"%s\")", net::CUtils::ipv4_tostring(_from_addr.sin_addr.s_addr).c_str(), label_str.c_str(), sys::CDatetimeUtils::to_datetime(_current_time).c_str());
                if (n != 1)
                {
                    _mysql->rollback();
                }
                else
                {
                    _mysql->commit();
                    label = static_cast<int>(atoi(label_str.c_str()));
                    break;
                }
            }
        }

        MYLOG_ERROR("no lable available for %s\n", net::to_string(_from_addr).c_str());
        return label;
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("[%s][%s]=>%s", net::to_string(_from_addr.sin_addr).c_str(), ex.sql(), ex.str().c_str());
        return 0;
    }
}

int CUniqMaster::get_label() const
{
    try
    {
        const std::string str = _mysql->query("SELECT f_label FROM t_label_online WHERE f_ip=\"%s\"", net::to_string(_from_addr.sin_addr).c_str());
        MYLOG_INFO("%s hold lable[%s]\n", net::to_string(_from_addr.sin_addr).c_str(), str.c_str());
        if (str.empty())
            return 0;
        return atoi(str.c_str());
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("[%s][%s]=>%s", net::to_string(_from_addr.sin_addr).c_str(), ex.sql(), ex.str().c_str());
        return -1;
    }
}

bool CUniqMaster::hold_label(uint8_t label) const
{
    const std::string str = _mysql->query("SELECT f_ip FROM t_label_online WHERE f_label=%d", (int)label);

    if (str.empty())
    {
        MYLOG_ERROR("%s not hold lable[%d]\n", net::to_string(_from_addr).c_str(), (int)label);
        return false;
    }
    else
    {
        return str == net::to_string(_from_addr.sin_addr);
    }
}

void CUniqMaster::on_timeout()
{
    // 更新续租
    for (std::map<uint8_t, struct LabelInfo>::iterator iter=_label_info_map.begin(); iter!=_label_info_map.end(); ++iter)
    {
        const struct LabelInfo& label_info = iter->second;

        try
        {
            _mysql->update("UPDATE t_label_online SET f_time=\"%s\" WHERE f_label=%d AND f_ip=\"%s\"", sys::CDatetimeUtils::to_datetime(label_info.lease_time).c_str(), label_info.label, net::ip2string(label_info.ip).c_str());
        }
        catch (sys::CDBException& ex)
        {
            MYLOG_ERROR("[%s]=>%s", ex.sql(), ex.str().c_str());
        }
    }

    // 清理超时未续租的
    try
    {
        time_t timeout_time = _current_time - argument::expire->value();

        sys::DBTable db_table;
        _mysql->query(db_table, "SELECT f_ip,f_label,f_time FROM t_label_online WHERE f_time<\"%s\"", sys::CDatetimeUtils::to_datetime(timeout_time).c_str());
        if (db_table.empty())
        {
            MYLOG_INFO("no lable timeout\n");
        }
        else
        {
            for (sys::DBTable::size_type row=0; row<db_table.size(); ++row)
            {
                int num_rows = 0;
                const sys::DBRow& db_row = db_table[row];
                const std::string& ip_str = db_row[0];
                const std::string& label_str = db_row[1];
                const std::string& time_str = db_row[2];

                num_rows = _mysql->update("DELETE FROM t_label_online WHERE f_label=%s", label_str.c_str());
                MYLOG_INFO("[%d] Label[%s] expired for %s with %s\n", num_rows, label_str.c_str(), ip_str.c_str(), time_str.c_str());

                // 回收
                num_rows = _mysql->update("INSERT INTO t_label (f_label) VALUES (%s)", label_str.c_str());
                MYLOG_INFO("Label[%s] recycled from %s\n", label_str.c_str(), ip_str.c_str());

                _mysql->commit();
            }
        }
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("[%s]=>%s", ex.sql(), ex.str().c_str());

        try
        {
            _mysql->rollback();
        }
        catch (sys::CDBException& ex)
        {
            MYLOG_ERROR("rollback failed: %s", ex.str().c_str());
        }
    }
}

void CUniqMaster::prepare_response_error(int errcode)
{
    struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);

    _response_size = sizeof(struct MessageHead);
    response->len = sizeof(struct MessageHead);
    response->type = RESPONSE_ERROR;
    response->echo = request->echo;
    response->value1 = errcode;
    response->value2 = 0;

    MYLOG_DEBUG("prepare %s ok for %s\n", response->str().c_str(), net::to_string(_from_addr).c_str());
}

int CUniqMaster::prepare_response_get_label()
{
    struct MessageHead* request = reinterpret_cast<struct MessageHead*>(_request_buffer);
    struct MessageHead* response = reinterpret_cast<struct MessageHead*>(_response_buffer);
    int label = static_cast<int>(request->value1.to_int());

    if ((label < 0) || (label > LABEL_MAX))
    {
        MYLOG_ERROR("invalid label[%u] from %s\n", label, net::to_string(_from_addr).c_str());
        return ERROR_INVALID_LABEL;
    }
    else
    {
        if (label > 0)
        {
            // 续租
            if (!hold_label(label))
                return ERROR_LABEL_NOT_HOLD;
        }
        else
        {
            label = get_label();
            if (-1 == label)
            {
                return ERROR_DATABASE;
            }
            else if (0 == label)
            {
                // 新租
                label = alloc_label();
                if (0 == label)
                {
                    return ERROR_NO_LABEL;
                }
            }
        }

        _response_size = sizeof(struct MessageHead);
        response->len = sizeof(struct MessageHead);
        response->type = RESPONSE_LABEL;
        response->echo = request->echo;
        response->value1 = label;
        response->value2 = 0;
        MYLOG_INFO("%s => %s\n", response->str().c_str(), net::to_string(_from_addr).c_str());

        struct LabelInfo label_info(label, _from_addr.sin_addr.s_addr, _current_time);
        _label_info_map.insert(std::make_pair(label, label_info));
    }

    return 0;
}

} // namespace mooon {
