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

// 允许两个master同时运行，
// 通过MySQL的事务、表锁和唯一键保证数据的安全和一致性。

// 日志控制：
// 可通过设置环境变量MOOON_LOG_LEVEL和MOOON_LOG_SCREEN来控制日志级别和是否在屏幕上输出日志
// 1) MOOON_LOG_LEVEL可以取值debug,info,error,warn,fatal
// 2) MOOON_LOG_SCREEN取值为1表示在屏幕输出日志，其它则表示不输出

STRING_ARG_DEFINE(ip, "0.0.0.0", "listen IP");
INTEGER_ARG_DEFINE(uint16_t, port, 16200, 1000, 65535, "listen port");

STRING_ARG_DEFINE(db_host, "", "MySQL host to connect");
INTEGER_ARG_DEFINE(uint16_t, db_port, 3306, 1000, 65535, "MySQL port to connect");
STRING_ARG_DEFINE(db_user, "root", "MySQL user");
STRING_ARG_DEFINE(db_pass, "", "MySQL password");
STRING_ARG_DEFINE(db_name, "", "MySQL database name");

// Label过期时长参数，所有节点的expire值必须保持相同，包括master节点和所有agent节点
//
// expire用来控制Label的回收重利用，取值应当越大越好，比如可以30天则取30天，可以取7天则7天等，
// 但是过期后并不会立即被回收，而是有一个冻结期，冻结期的时间长短和expire的值相关。
//
// 当一个Label在expire指定的时间内都没有续租赁过，则会进入一段冻结期，冻结期内该Label不会被回收，但也不能被租赁，
// 在冻结期之后，该Label则会被回收进入f_label_pool表中。
INTEGER_ARG_DEFINE(uint32_t, expire, LABEL_EXPIRED_SECONDS, 1, 4294967295U, "label expired seconds");
// 多长间隔做一次Label过期检测和处理过冻结期的Labels，参数取值应当要小于agent的interval值，比如可以小一半
// 如果expire的值为1天，则timeout的值可以取10分钟，
// 如果expire的值为7天，则timeout的值可以取1小时，
// 如果expire的值为30天，则timeout的值可以取12小时
INTEGER_ARG_DEFINE(uint32_t, timeout, 3600, 1, 36000, "timeout seconds");

////////////////////////////////////////////////////////////////////////////////
namespace mooon {

// CREATE DATABASE IF NOT EXISTS uniq_id DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

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
 * f_label为主键，保证同一时间不可能有两个IP租赁同一个Label，
 * 但一个IP可能同时持有一个或多个过期的Label，和一个当前有效的Label
DROP TABLE IF EXISTS t_label_online;
CREATE TABLE t_label_online (
    f_label TINYINT UNSIGNED NOT NULL,
    f_ip CHAR(16) NOT NULL,
    f_time DATETIME NOT NULL,
    PRIMARY KEY (f_label),
    KEY idx_ip(f_ip),
    KEY idx_time(f_time)
);
*/

/*
 * Label日志表
DROP TABLE IF EXISTS t_label_log;
CREATE TABLE t_label_log (
    f_id BIGINT NOT NULL AUTO_INCREMENT,
    f_label TINYINT UNSIGNED NOT NULL,
    f_ip CHAR(16) NOT NULL,
    f_event TINYINT NOT NULL,
    f_time DATETIME NOT NULL,
    PRIMARY KEY (f_id),
    KEY idx_label(f_label),
    KEY idx_ip(f_ip),
    KEY idx_event(f_event),
    KEY idx_time(f_time)
);
*/

// 常量
enum
{
    LABEL_RENTED = 1,  // Label被租赁
    LABEL_RECYCLED = 2 // Label被回收
};

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
        return mooon::utils::CStringUtils::format_string("label://L%d/%s/%s", (int)label, inet_ntoa(in), sys::CDatetimeUtils::to_datetime(lease_time).c_str());
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
    bool hold_valid_label(uint8_t label) const;

private:
    void on_timeout();
    time_t get_expire_time() const;
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
    time_t old_time = 0;

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
        _mysql->enable_autocommit(false); // 为false表示开启事务
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
    bool need_rollback = false;

    try
    {
        std::string str = _mysql->query("SELECT count(1) FROM t_label_pool");
        if (str == "0")
        {
            for (int label=0; label<=LABEL_MAX; ++label)
            {
                _mysql->update("INSERT INTO t_label_pool (f_label) VALUES (%d)", label);
                need_rollback = true;
            }

            _mysql->commit();
            need_rollback = false;
            MYLOG_INFO("generate labels ok\n");
        }

        return true;
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("generate labels failed: %s\n", ex.str().c_str());

        if (need_rollback)
        {
            try
            {
                _mysql->rollback();
            }
            catch (sys::CDBException& ex)
            {
                MYLOG_ERROR("rollback failed: %s\n", ex.str().c_str());
            }
        }

        return false;
    }
}

// 返回-1表示DB错误
// 返回0表示无可用的Label
// 返回大于0的值表示分配Label成功
int CUniqMaster::alloc_label()
{
    bool need_rollback = false;

    try
    {
        const std::string label_str = _mysql->query("SELECT f_label FROM t_label_pool WHERE f_label<=%d AND f_label>0 LIMIT 1", LABEL_MAX);
        if (label_str.empty())
        {
            MYLOG_ERROR("no label available for %s\n", net::to_string(_from_addr).c_str());
            return 0;
        }

        // 被其它master抢走时，返回值为0
        int n = _mysql->update("DELETE FROM t_label_pool WHERE f_label=%s", label_str.c_str());
        if (n != 1)
        {
            MYLOG_ERROR("Label[%s] return %d\n", label_str.c_str(), n);
            if (n > 0)
                _mysql->rollback();
            return -1;
        }
        else
        {
            const std::string ip_str = net::CUtils::ipv4_tostring(_from_addr.sin_addr.s_addr);
            const std::string time_str = sys::CDatetimeUtils::to_datetime(_current_time);

            need_rollback = true;
            n = _mysql->update("INSERT INTO t_label_online (f_label,f_ip,f_time) VALUES (%s,\"%s\",\"%s\")", label_str.c_str(), ip_str.c_str(), time_str.c_str());
            if (n != 1)
            {
                MYLOG_ERROR("Label[%s] return %d\n", label_str.c_str(), n);
                _mysql->rollback();
                return -1;
            }
            else
            {
                _mysql->update("INSERT INTO t_label_log(f_label,f_ip,f_event,f_time) VALUES (%s,\"%s\",%d,\"%s\")", label_str.c_str(), ip_str.c_str(), LABEL_RENTED, time_str.c_str());

                MYLOG_DEBUG("Label[%s] return %d\n", label_str.c_str(), n);
                _mysql->commit();
                return static_cast<int>(atoi(label_str.c_str()));
            }
        }
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("%s\n", ex.str().c_str());

        if (need_rollback)
        {
            try
            {
                _mysql->rollback();
            }
            catch (sys::CDBException& ex)
            {
                MYLOG_ERROR("rollback failed: %s\n", ex.str().c_str());
            }
        }

        return -1;
    }
}

// 根据IP取它的未过期的Label
// 成功返回Lable， 没找到则返回0，DB错误返回-1
int CUniqMaster::get_label() const
{
    try
    {
        time_t expire_time = _current_time - argument::expire->value();
        const std::string str = _mysql->query("SELECT f_label FROM t_label_online WHERE f_ip=\"%s\" AND f_time>=\"%s\"", net::to_string(_from_addr.sin_addr).c_str(), sys::CDatetimeUtils::to_datetime(expire_time).c_str());

        if (str.empty())
        {
            MYLOG_INFO("%s not hold valid label\n", net::to_string(_from_addr.sin_addr).c_str());
            return 0;
        }
        else
        {
            MYLOG_INFO("%s hold label[%s]\n", net::to_string(_from_addr.sin_addr).c_str(), str.c_str());
            return atoi(str.c_str());
        }
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("[%s][%s]=>%s", net::to_string(_from_addr.sin_addr).c_str(), ex.sql(), ex.str().c_str());
        return -1;
    }
}

// 判断是否持有指定的Label，而且需要在有效期内
bool CUniqMaster::hold_valid_label(uint8_t label) const
{
    time_t expire_time = _current_time - argument::expire->value();
    const std::string str = _mysql->query("SELECT f_ip FROM t_label_online WHERE f_label=%d AND f_time>=\"%s\"", (int)label, sys::CDatetimeUtils::to_datetime(expire_time).c_str());

    if (!str.empty())
    {
        return str == net::to_string(_from_addr.sin_addr);
    }
    else
    {
        MYLOG_ERROR("%s not hold label[%d] or label expired\n", net::to_string(_from_addr).c_str(), (int)label);
        return false;
    }
}

void CUniqMaster::on_timeout()
{
    int num_rows = 0;
    bool need_commit = false;
    bool need_rollback = false; // 是否需要回滚

    try
    {
        // 需先更新续租
        for (std::map<uint8_t, struct LabelInfo>::iterator iter=_label_info_map.begin(); iter!=_label_info_map.end(); ++iter)
        {
            try
            {
                const struct LabelInfo& label_info = iter->second;
                const std::string ip_str = net::ip2string(label_info.ip);
                const std::string time_str = sys::CDatetimeUtils::to_datetime(label_info.lease_time);

                num_rows = _mysql->update("UPDATE t_label_online SET f_time=\"%s\" WHERE f_label=%d AND f_ip=\"%s\" AND f_time<\"%s\"", time_str.c_str(), (int)label_info.label, ip_str.c_str(), time_str.c_str());
                MYLOG_DEBUG("update t_label_online ok: (%d)%s\n", num_rows, label_info.str().c_str());
                need_commit = true;
            }
            catch (sys::CDBException& ex)
            {
                MYLOG_ERROR("[%s]=>%s", ex.sql(), ex.str().c_str());
            }
        }

        if (need_commit)
        {
            _mysql->commit();
            need_commit = false;
            _label_info_map.clear();
        }
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("commit failed: %s", ex.str().c_str());
    }

    // 再清理超时未续租的，否则可能冲突
    try
    {
        static uint64_t timeout_count = 0; // 控制重复日志量
        time_t expire_time = get_expire_time();

        sys::DBTable db_table;
        _mysql->query(db_table, "SELECT f_ip,f_label,f_time FROM t_label_online WHERE f_time<\"%s\"", sys::CDatetimeUtils::to_datetime(expire_time).c_str());
        if (db_table.empty())
        {
            if (0 == timeout_count++ % 10000)
            {
                MYLOG_INFO("no label timeout\n");
            }
        }
        else
        {
            timeout_count = 0;

            // 锁住表: LOCK TABLES t_label_online WRITE;
            // 开始事务: START TRANSACTION;
            // 事务包含了锁操作
            //_mysql->update("%s", "LOCK TABLES t_label_online WRITE");

            for (sys::DBTable::size_type row=0; row<db_table.size(); ++row)
            {
                const sys::DBRow& db_row = db_table[row];
                const std::string& ip_str = db_row[0];
                const std::string& label_str = db_row[1];
                const std::string& time_str = db_row[2];

                // 如果另一连接进入了事务，则update会被阻塞
                // 如果一个连接已完成了DELETE，则调用后返回值为0
                num_rows = _mysql->update("DELETE FROM t_label_online WHERE f_label=%s", label_str.c_str());
                if (0 == num_rows)
                {
                    need_rollback = false;
                    MYLOG_WARN("Label[%s] not exist in `t_label_online`\n", label_str.c_str());
                }
                else
                {
                    need_rollback = true;
                    MYLOG_INFO("[%d] Label[%s] expired(%u) for %s with %s\n", num_rows, label_str.c_str(), argument::expire->value(), ip_str.c_str(), time_str.c_str());

                    // 回收
                    num_rows = _mysql->update("INSERT INTO t_label_pool (f_label) VALUES (%s)", label_str.c_str());
                    // 日志
                    _mysql->update("INSERT INTO t_label_log(f_label,f_ip,f_event,f_time) VALUES (%s,\"%s\",%d,\"%s\")", label_str.c_str(), ip_str.c_str(), LABEL_RECYCLED, sys::CDatetimeUtils::to_datetime(_current_time).c_str());
                    MYLOG_INFO("Label[%s] recycled from %s, expired at %s\n", label_str.c_str(), ip_str.c_str(), time_str.c_str());

                    _mysql->commit();
                }
            } // for
        }
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("[%s]=>%s", ex.sql(), ex.str().c_str());

        // 回滚
        if (need_rollback)
        {
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
}

// 一个Label过期未续租时，不是立即回收，
// 而是在过期后仍然冻结一段时间，以保证回收的足够安全，这有点类似于TCP的TIME_WAIT状态
time_t CUniqMaster::get_expire_time() const
{
    time_t  expire_time;

    if (argument::expire->value() < 3600) // 小于1小时，则冻结时间为过期时间的2倍，此种情况主要用于测试
    {
        expire_time = _current_time - (2 * argument::expire->value());
    }
    else if (argument::expire->value() < 3600*24) // 小于一天，则冻结时间为过期时间加2小时，此种情况主要用于测试
    {
        expire_time = _current_time - (3600*2 + argument::expire->value());
    }
    else if (argument::expire->value() < 3600*24*7) // 小于一周，则冻结时间为过期时间加上2天
    {
        expire_time = _current_time - ((3600*24*2) + argument::expire->value());
    }
    else if (argument::expire->value() < 3600*24*30) // 过期时间小于1个月，则冻结时间加上3天
    {
        expire_time = _current_time - ((3600*24*3) + argument::expire->value());
    }
    else if (argument::expire->value() < 3600*24*90) // 过期时间小于3个月，则冻结时间加上5天
    {
        expire_time = _current_time - ((3600*24*5) + argument::expire->value());
    }
    else // 冻结时间加上10天
    {
        expire_time = _current_time - ((3600*24*10) + argument::expire->value());
    }

    return expire_time;
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
            // 续租，如果agent不持有该label，则应当重新租赁
            if (!hold_valid_label(label))
                return ERROR_LABEL_NOT_HOLD;
        }
        else
        {
            // 新租赁Label，
            // 先查看是否已持有Label
            label = get_label();
            if (-1 == label)
            {
                return ERROR_DATABASE;
            }
            else if (0 == label)
            {
                // 新租赁
                label = alloc_label();
                if (0 == label)
                {
                    return ERROR_NO_LABEL;
                }
                else if (-1 == label)
                {
                    return ERROR_DATABASE;
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
        std::pair<std::map<uint8_t, struct LabelInfo>::iterator, bool> ret = _label_info_map.insert(std::make_pair(label, label_info));
        if (!ret.second)
        {
            struct LabelInfo& label_info_ref = ret.first->second;
            label_info_ref.lease_time = _current_time;
        }
    }

    return 0;
}

} // namespace mooon {
