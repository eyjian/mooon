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
#ifndef MOOON_SYS_MYSQL_DB_H
#define MOOON_SYS_MYSQL_DB_H
#include "mooon/sys/simple_db.h"
#include "mooon/utils/object.h"
#include <stdarg.h>

#if MOOON_HAVE_MYSQL==1
SYS_NAMESPACE_BEGIN

/**
 * MySQL版本的DB连接
 */
class CMySQLConnection: public CDBConnectionBase, public utils::CObject
{
public:
    // Call this function to initialize the MySQL library before you call any other MySQL function
    // 多线程环境必须调用library_init，并且必须在创建任何线程之前调用library_init
    static bool library_init(int argc=0, char **argv=NULL, char **groups=NULL);
    // library_end用于释放library_init分配的资源
    static void library_end();

    static bool is_duplicate(int errcode);

    // 双引号转成：\"
    // 单引号转成：\'
    // 单斜杠转成双斜杠
    // 注意不转义空格、|、?、<、>、{、}、:、~、@、!、(、)、`、#、%、,、;、&、-和_等
    static void escape_string(const std::string& str, std::string* escaped_str);

public:
    CMySQLConnection(size_t sql_max=8192);
    ~CMySQLConnection();
    void* get_mysql_handle() const { return _mysql_handle; }

public:
    virtual bool is_syntax_exception(int errcode) const; // errcode值为1064
    virtual bool is_duplicate_exception(int errcode) const; // errcode值为1062
    virtual bool is_disconnected_exception(CDBException& db_error) const;
    virtual bool is_deadlock_exception(CDBException& db_error) const;
    virtual bool is_shutdowning_exception(CDBException& db_error) const;

    // 如果一条查询语句过慢，可能导致大量的lost错误消耗光MySQL连接
    // 因此遇到此错误时，需要考虑降慢重连接速度，即使开启了自动得连接！
    virtual bool is_lost_connection_exception(CDBException& db_error) const;
    virtual bool is_too_many_connections_exception(CDBException& db_error) const;

    virtual bool is_query_interrupted_exception(CDBException& db_error) const;
    virtual bool is_connect_host_exception(CDBException& db_error) const;
    virtual bool is_server_gone_exception(CDBException& db_error) const;
    virtual bool is_connection_error_exception(CDBException& db_error) const;
    virtual bool is_server_handshake_exception(CDBException& db_error) const;
    virtual bool is_ipsock_exception(CDBException& db_error) const;

    virtual std::string escape_string(const std::string& str) const throw (CDBException);
    virtual void change_charset(const std::string& charset) throw (CDBException);
    virtual void open() throw (CDBException);
    virtual void close() throw ();
    virtual void reopen() throw (CDBException);

    // 如果update的值并没变化返回0，否则返回变修改的行数
    virtual uint64_t update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));
    virtual uint64_t get_insert_id() const;
    virtual std::string str() throw ();

    virtual void ping() throw (CDBException);
    virtual void commit() throw (CDBException);
    virtual void rollback() throw (CDBException);

    /** 是否允许自动提交事务，注意只有open()或reopen()成功之后，才可以调用 */
    virtual void enable_autocommit(bool enabled) throw (CDBException);

private:
    virtual void do_query(DBTable& db_table, const char* sql, int sql_length) throw (CDBException);

private:
    void do_open() throw (CDBException);

private:
    void* _mysql_handle; // MySQL句柄
};

SYS_NAMESPACE_END
#endif // MOOON_HAVE_MYSQL
#endif // MOOON_SYS_MYSQL_DB_H
