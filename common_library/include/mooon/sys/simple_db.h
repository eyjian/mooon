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
    db->set_host("127.0.0.1", 3600);
    db->set_user("root", "");
    db->set_charset("utf8");
    db->enable_auto_reconnect();
    db->set_timeout_seconds(600);
    
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
    /***
     * 工厂方法 - 创建一个DB连接
     * @db_type_name DB类型名，如：mysql、oracle、postgreSQL，不区别大小写
     *               当前只支持MySQL，也就是参数值只能输入mysql（不区别大小写）
     * @sql_max 支持的最大SQL语句长度，单位为字节数，不含结尾符
     * 如果是支持的DB类型，则返回非NULL，否则返回NULL
     */
    static DBConnection* create_connection(const std::string& db_type_name, size_t sql_max);

    /***
     * 销毁一个由create_connection()创建的DB连接
     * @db_connection 需要销毁的DB连接
     */
    static void destroy_connection(DBConnection* db_connection);

    /***
     * 判断是否为网络连接断开异常，
     * 如使用过程中，与MySQL间的网络中断，或MySQL进程死掉等，这种情况下可以尝试重连接
     */
    static bool is_disconnected_exception(CDBException& db_error);

public:
    virtual ~DBConnection() {}
    
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
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SIMPLE_DB_H
