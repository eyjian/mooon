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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_SYS_SIMPLE_DB_H
#define MOOON_SYS_SIMPLE_DB_H
#include "mooon/sys/db_exception.h"
#include <string>
#include <vector>
SYS_NAMESPACE_BEGIN

typedef std::vector<std::string> DBRow; // 用来存储一行所有字段的值
typedef std::vector<DBRow> DBTable;     // 用来存储所有行

// DB连接、读和写超时时长，单位为秒
enum
{
    DB_CONNECT_TIMEOUT_SECONDS_DEFAULT = 2,
    DB_READ_TIMEOUT_SECONDS_DEFAULT = 2, // MySQL对读有重试三次机制，因此实际的读超时时长可能为三倍
    DB_WRITE_TIMEOUT_SECONDS_DEFAULT = 2 // MySQL对写有重试两次机制，因此实际的读超时时长可能为两倍
};

/**
 * 访问DB的接口，是一个抽象接口，当前只支持MySQL
 *
 * 使用示例：
#include <mooon/sys/mysql_db.h>
DBConnection* db_connection = new CMySQLConnection;
try
{
    DBTable db_table;

    // 不指定DB名，以及不需要密码
    db_connection->set_host("127.0.0.1", 3600);
    db_connection->set_user("root", "");
    db_connection->set_charset("utf8");
    db_connection->enable_auto_reconnect();
    db_connection->set_timeout_seconds(600);
    //db_connection->set_db_name("test");
    
    db_connection->open();
    db_connection->query(db_table, "select count(1) from test_table where id=%d", 2015);
}
catch (CDBException& db_error)
{
    log("%s error: %s at %s:%d", db_connection.str().c_str(), db_error.what(), db_error.file(), db_error.lien());
}

delete db_connection;
 */

// 请不要跨线程使用
class DBConnection
{
public:
    virtual ~DBConnection() {}
    
    // 是否为语法错误
    virtual bool is_syntax_exception(int errcode) const { return false; }

    // 是否为重复记录
    virtual bool is_duplicate_exception(int errcode) const { return false; }

    // 判断是否为断开连接异常
    virtual bool is_disconnected_exception(CDBException& db_error) const { return false; }

    // 是否发生死锁错误，
    // 对于MySQL为：ER_LOCK_DEADLOCK(1213) Deadlock found when trying to get lock; try restarting transaction
    virtual bool is_deadlock_exception(CDBException& db_error) const { return false; }

    // 关闭中
    // 对于MySQL为：ER_SERVER_SHUTDOWN(1053) Server shutdown in progress 当执行关闭MySQL时
    virtual bool is_shutdowning_exception(CDBException& db_error) const { return false; }

    // Lost connection to MySQL server during query
    virtual bool is_lost_connection_exception(CDBException& db_error) const { return false; }

    // Too many connections
    virtual bool is_too_many_connections(CDBException& db_error) const { return false; }

    virtual bool is_query_interrupted_exception(CDBException& db_error) const { return false; }
    virtual bool is_connect_host_exception(CDBException& db_error) const { return false; }
    virtual bool is_server_gone_exception(CDBException& db_error) const { return false; }
    virtual bool is_connection_error_exception(CDBException& db_error) const { return false; }
    virtual bool is_server_handshake_exception(CDBException& db_error) const { return false; }
    virtual bool is_ipsock_exception(CDBException& db_error) const { return false; }

    // 对字符串进行编码，以防止SQL注入
    // str 需要编码的字符串，返回被编码后的字符串
    virtual std::string escape_string(const std::string& str) const throw (CDBException) = 0;

    /***
     * 设置需要连接的DB的IP和服务端口号
     * 注意，只有在open()或reopen()之前调用才生效
     */
    virtual void set_host(const std::string& db_ip, uint16_t db_port) = 0;

    /***
     * 设置连接的数据库名
     * 注意，只有在open()或reopen()之前调用才生效
     */
    virtual void set_db_name(const std::string& db_name) = 0;
    
    /***
     * 设置用来连接DB的用户名和密码
     * 注意，只有在open()或reopen()之前调用才生效
     */
    virtual void set_user(const std::string& db_user, const std::string& db_password) = 0;
    
    /***
     * 设置访问DB的字符集
     * 注意，只有在open()或reopen()之前调用才生效
     */
    virtual void set_charset(const std::string& charset) = 0;
    
    /***
     * 设置为连接断开后自动重连接，如果不主动设置，默认不自动重连接
     * 注意，只有在open()或reopen()之前调用才生效
     */
    virtual void enable_auto_reconnect(bool auto_reconnect=true) = 0;
    
    /***
     * 设置连接、读和写超时时长
     * 如果read_timeout_seconds值为负值，则表示使用和connect_timeout_seconds相同的值，
     * 如果write_timeout_seconds值为负值，则表示使用和connect_timeout_seconds相同的值，
     */
    virtual void set_timeout_seconds(int connect_timeout_seconds, int read_timeout_seconds=-1, int write_timeout_seconds=-1) = 0;

    /***
     * 设置用来连接的超时秒数，如果不主动设置则使用默认的DB_CONNECT_TIMEOUT_SECONDS_DEFAULT秒
     * 注意，只有在open()或reopen()之前调用才生效
     *
     * 对于读MySQL有重试三次机制，因此实际超时时长可能为参数timeout_seconds的三倍，
     * 对于写MySQL有重试两次机制，因此实际超时时长可能为参数timeout_seconds的两倍
     */
    virtual void set_connect_timeout_seconds(int timeout_seconds) = 0;

    /***
     * 设置读超时，如果不主动设置则使用默认的DB_READ_TIMEOUT_SECONDS_DEFAULT秒
     * 对于读MySQL有重试三次机制，因此实际超时时长可能为参数timeout_seconds的三倍
     */
    virtual void set_read_timeout_seconds(int timeout_seconds) = 0;

    /***
     * 设置写超时，如果不主动设置则使用默认的DB_WRITE_TIMEOUT_SECONDS_DEFAULT秒
     * 对于写MySQL有重试两次机制，因此实际超时时长可能为参数timeout_seconds的两倍
     */
    virtual void set_write_timeout_seconds(int timeout_seconds) = 0;
    
    /***
     * 设置空值，字段在DB表中的值为NULL时，返回的内容
     * 如果不主动设置，则默认空值时被设置为"$NULL$"。
     */
    virtual void set_null_value(const std::string& null_value) = 0;
    
    /***
     * 设置或修改连接的字符集
     * 如果charset为空，则什么也不做
     */
    virtual void change_charset(const std::string& charset) throw (CDBException) {}

    /***
     * 建立一个DB连接
     * 出错抛出异常CDBException
     */
    virtual void open() throw (CDBException) = 0;

    /**
     * 关闭一个DB连接
     * 使用open()建立的连接，在使用完后，要使用close()关闭它
     */
    virtual void close() throw () = 0;

    /***
     * 重新建立DB连接
     * reopen()会先调用close()关闭连接，然后才重新建立连接，
     * 因此调用reopen()之前，可不调用close()，当然即使调用了close()也不会有问题
     */
    virtual void reopen() throw (CDBException) = 0;

    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等，
      * 如果某字段在DB表中为NULL，则返回结果为"$NULL$"，如果内容刚好为"$NULL$"，则无法区分
      * 出错抛出CDBException异常
      *
      * 特别注意：
      * 如果需要向DB执行一个带格式化的查询，比如：
      * select date_format(now(), '%Y%m')
      * 则传递给query的参数应当为：
      * select date_format(now(), '%%Y%%m')
      * 注意使用了双%号，否则像“%m”会被query内部调用的vsnprintf解析成它自身内置的“%m”，
      * 比如预期结果为201611，实际返回结果变成了2016success，就是因为这个原因！
      */
    virtual void query(DBTable& db_table, const char* format, ...) throw (CDBException) __attribute__((format(printf, 3, 4))) = 0;

    /**
     * 查询，期望只返回一行记录，
     * 如果某字段在DB表中为NULL，则返回结果为空字符串，因此不能区分字段无值还是值为空字符串
     * 如果查询失败，抛出CDBException异常，异常的错误码为-1，
     * 如果查询实际返回超过一行记录，抛出CDBException异常，异常的错误码为DB_ERROR_TOO_MANY_ROWS
     */
    virtual void query(DBRow& db_row, const char* format, ...) throw (CDBException) __attribute__((format(printf, 3, 4))) = 0;

    /**
     * 查询，期望只返回单行单列，
     * 如果某字段在DB表中为NULL，则返回结果为空字符串，因此不能区分字段无值还是值为空字符串
     * 如果查询失败，抛出CDBException异常，异常的错误码为-1，
     * 如果查询实际返回超过一行记录，抛出CDBException异常，异常的错误码为DB_ERROR_TOO_MANY_ROWS，
     * 如果查询实际返回只有一行，但超过一列，则抛出CDBException异常，异常的错误码为DB_ERROR_TOO_MANY_COLS
     */
    virtual std::string query(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3))) = 0;

    /***
      * 数据库insert和update更新操作
      * 对于MySQL如果update的值并没变化返回0，否则返回变修改的行数
      * 出错则抛出CDBException异常
      */
    virtual uint64_t update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3))) = 0;

    /***
     * 取得insert_id，对MySQL有效
     */
    virtual uint64_t get_insert_id() const { return 0; }

    /**
     * 取得可读的字符串信息
     */
    virtual std::string str() throw () = 0;

    virtual void ping() throw (CDBException) = 0;
    virtual void commit() throw (CDBException) = 0;
    virtual void rollback() throw (CDBException) = 0;

    /** 是否允许自动提交事务，注意只有open()或reopen()成功之后，才可以调用 */
    virtual void enable_autocommit(bool enabled) throw (CDBException) = 0;

    /** 是否已建立DB连接 */
    virtual bool is_established() const = 0;
};


/**
 * 不同DB的通用操作，请不要跨线程使用
 */
class CDBConnectionBase: public DBConnection
{
public:
    CDBConnectionBase(size_t sql_size);

public:
    virtual std::string escape_string(const std::string& str) const  throw (CDBException) { return str; }
    virtual void set_host(const std::string& db_ip, uint16_t db_port);
    virtual void set_db_name(const std::string& db_name);
    virtual void set_user(const std::string& db_user, const std::string& db_password);
    virtual void set_charset(const std::string& charset);
    virtual void enable_auto_reconnect(bool auto_reconnect=true);
    virtual void set_timeout_seconds(int connect_timeout_seconds, int read_timeout_seconds=-1, int write_timeout_seconds=-1);
    virtual void set_connect_timeout_seconds(int timeout_seconds);
    virtual void set_read_timeout_seconds(int timeout_seconds);
    virtual void set_write_timeout_seconds(int timeout_seconds);
    virtual void set_null_value(const std::string& null_value);

public:
    virtual void query(DBTable& db_table, const char* format, ...) throw (CDBException) __attribute__((format(printf, 3, 4)));
    virtual void query(DBRow& db_row, const char* format, ...) throw (CDBException) __attribute__((format(printf, 3, 4)));
    virtual std::string query(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));

    virtual void ping() throw (CDBException);
    virtual void commit() throw (CDBException);
    virtual void rollback() throw (CDBException);

    /** 是否允许自动提交事务，注意只有open()或reopen()成功之后，才可以调用 */
    virtual void enable_autocommit(bool enabled) throw (CDBException);

    /** 是否已建立DB连接 */
    virtual bool is_established() const;

private:
    virtual void do_query(DBTable& db_table, const char* sql, int sql_length) throw (CDBException) = 0;

protected:
    const size_t _sql_size; // 支持的最大SQL语句长度，单位为字节数，不含结尾符
    bool _is_established;  // 是否已经和数据库建立的连接
    std::string _id;       // 身份标识，用来识别

protected:
    std::string _db_ip;
    uint16_t _db_port;
    std::string _db_name;
    std::string _db_user;
    std::string _db_password;
    std::string _charset;
    bool _auto_reconnect;
    int _connect_timeout_seconds; // 连接超时
    int _read_timeout_seconds;
    int _write_timeout_seconds;
    std::string _null_value; // 字段在DB表中的值为NULL时，返回的内容
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SIMPLE_DB_H
