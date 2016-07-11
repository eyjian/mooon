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

#if HAVE_MYSQL==1
SYS_NAMESPACE_BEGIN

/**
 * MySQL版本的DB连接
 */
class CMySQLConnection: public CDBConnectionBase, public utils::CObject
{
public:
    static bool is_duplicate(int errcode);

public:
    CMySQLConnection(size_t sql_max=8192);
    ~CMySQLConnection();

public:
    virtual bool is_syntax_exception(int errcode) const; // errcode值为1064
    virtual bool is_duplicate_exception(int errcode) const; // errcode值为1062
    virtual bool is_disconnected_exception(CDBException& db_error) const;
    virtual std::string escape_string(const std::string& str) const;
    virtual void open() throw (CDBException);
    virtual void close() throw ();
    virtual void reopen() throw (CDBException);

    virtual int update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));
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
    void* _mysql_handler; // MySQL句柄
};

SYS_NAMESPACE_END
#endif // HAVE_MYSQL
#endif // MOOON_SYS_MYSQL_DB_H
