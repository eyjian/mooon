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

/**
 * 访问DB的接口，是一个抽象接口，当前只支持MySQL
 *
 * 使用示例：
DBConnection* db_connection = DBConnection::create_connection("mysql");
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

DBConnection::destroy_connection(db_connection);
 */
class DBConnection
{
public:
    virtual ~DBConnection() {}
    
    // 对字符串进行编码，以防止SQL注入
    // str 需要编码的字符串，返回被编码后的字符串
    virtual std::string escape_string(const std::string& str) const = 0;

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
    virtual void enable_auto_reconnect() = 0;
    
    /***
     * 设置用来连接的超时秒数，如果不主动设置，则使用默认的10秒
     * 注意，只有在open()或reopen()之前调用才生效
     */
    virtual void set_timeout_seconds(int timeout_seconds) = 0;
    
    /***
     * 设置空值，字段在DB表中的值为NULL时，返回的内容
     * 如果不主动设置，则默认空值时被设置为"$NULL$"。
     */
    virtual void set_null_value(const std::string& null_value) = 0;
    
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
      * 成功返回受影响的记录个数，出错则抛出CDBException异常
      */
    virtual int update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3))) = 0;

    /**
     * 取得可读的字符串信息
     */
    virtual std::string str() throw () = 0;

    virtual void ping() throw (CDBException) = 0;
    virtual void commit() throw (CDBException) = 0;
    virtual void rollback() throw (CDBException) = 0;
    virtual void enable_autocommit(bool enabled) throw (CDBException) = 0;
};


/**
 * 不同DB的通用操作
 */
class CDBConnectionBase: public DBConnection
{
public:
    CDBConnectionBase(size_t sql_size);

private:
    virtual std::string escape_string(const std::string& str) const { return str; }
    virtual void set_host(const std::string& db_ip, uint16_t db_port);
    virtual void set_db_name(const std::string& db_name);
    virtual void set_user(const std::string& db_user, const std::string& db_password);
    virtual void set_charset(const std::string& charset);
    virtual void enable_auto_reconnect();
    virtual void set_timeout_seconds(int timeout_seconds);
    virtual void set_null_value(const std::string& null_value);

private:
    virtual void query(DBTable& db_table, const char* format, ...) throw (CDBException) __attribute__((format(printf, 3, 4)));
    virtual void query(DBRow& db_row, const char* format, ...) throw (CDBException) __attribute__((format(printf, 3, 4)));
    virtual std::string query(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));

    virtual void ping() throw (CDBException);
    virtual void commit() throw (CDBException);
    virtual void rollback() throw (CDBException);
    virtual void enable_autocommit(bool enabled) throw (CDBException);

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
    int _timeout_seconds;
    std::string _null_value; // 字段在DB表中的值为NULL时，返回的内容
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SIMPLE_DB_H
