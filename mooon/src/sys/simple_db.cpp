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
#include "sys/simple_db.h"
#include "sys/mysql_db.h"
#include "sys/sqlite3_db.h"
#include "utils/scoped_ptr.h"
#include "utils/string_formatter.h"
#include "utils/string_utils.h"
#include <stdarg.h>
#include <strings.h>
SYS_NAMESPACE_BEGIN

CDBConnectionBase::CDBConnectionBase(size_t sql_size)
    : _sql_size(sql_size), _is_established(false),
      _db_port(3306), _auto_reconnect(false),
      _connect_timeout_seconds(DB_CONNECT_TIMEOUT_SECONDS_DEFAULT), _read_timeout_seconds(DB_READ_TIMEOUT_SECONDS_DEFAULT), _write_timeout_seconds(DB_WRITE_TIMEOUT_SECONDS_DEFAULT),
      _null_value("$NULL$")
{
}

void CDBConnectionBase::set_host(const std::string& db_ip, uint16_t db_port)
{
    _db_ip = db_ip;
    _db_port = db_port;    
}

void CDBConnectionBase::set_db_name(const std::string& db_name)
{
    _db_name = db_name;    
}

void CDBConnectionBase::set_user(const std::string& db_user, const std::string& db_password)
{
    _db_user = db_user;
    _db_password = db_password;    
}

void CDBConnectionBase::set_charset(const std::string& charset)
{
    _charset = charset;    
}

void CDBConnectionBase::enable_auto_reconnect(bool auto_reconnect)
{
    _auto_reconnect = auto_reconnect;
}

void CDBConnectionBase::set_timeout_seconds(int connect_timeout_seconds, int read_timeout_seconds, int write_timeout_seconds)
{
    // 连接超时
    set_connect_timeout_seconds(connect_timeout_seconds);

    // 读超时
    if (read_timeout_seconds < 0)
        set_read_timeout_seconds(connect_timeout_seconds);
    else
        set_read_timeout_seconds(read_timeout_seconds);

    // 写超时
    if (write_timeout_seconds < 0)
        set_write_timeout_seconds(connect_timeout_seconds);
    else
        set_write_timeout_seconds(write_timeout_seconds);
}

void CDBConnectionBase::set_connect_timeout_seconds(int timeout_seconds)
{
    _connect_timeout_seconds = timeout_seconds;
}

void CDBConnectionBase::set_read_timeout_seconds(int timeout_seconds)
{
    _read_timeout_seconds = timeout_seconds;
}

void CDBConnectionBase::set_write_timeout_seconds(int timeout_seconds)
{
    _write_timeout_seconds = timeout_seconds;
}

void CDBConnectionBase::set_null_value(const std::string& null_value)
{
    _null_value = null_value;
}
    
void CDBConnectionBase::query(DBTable& db_table, const char* format, ...) throw (CDBException)
{
    int excepted = 0;
    size_t sql_size = _sql_size;
    utils::ScopedArray<char> sql(new char[sql_size]);
    va_list ap;

    while (true)
    {
        va_start(ap, format);
        excepted = vsnprintf(sql.get(), sql_size, format, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (excepted > -1 && excepted < (int)sql_size)
            break;

        /* Else try again with more space. */
        if (excepted > -1)    /* glibc 2.1 */
            sql_size = (size_t)excepted + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            sql_size *= 2;  /* twice the old size */

        sql.reset(new char[sql_size]);
    }

    do_query(db_table, sql.get(), excepted);
}

void CDBConnectionBase::query(DBRow& db_row, const char* format, ...) throw (CDBException)
{
    DBTable db_table;
    int excepted = 0;
    size_t sql_size = _sql_size;
    utils::ScopedArray<char> sql(new char[sql_size]);
    va_list ap;

    while (true)
    {
        va_start(ap, format);
        excepted = vsnprintf(sql.get(), sql_size, format, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (excepted > -1 && excepted < (int)sql_size)
            break;

        /* Else try again with more space. */
        if (excepted > -1)    /* glibc 2.1 */
            sql_size = (size_t)excepted + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            sql_size *= 2;  /* twice the old size */

        sql.reset(new char[sql_size]);
    }

    do_query(db_table, sql.get(), excepted);
    if (1 == db_table.size())
    {
        db_row = db_table.front();
    }
    else if (db_table.size() > 1)
    {
        throw CDBException(sql.get(), utils::StringFormatter("too many rows: %d", (int)db_table.size()).c_str(),
                DB_ERROR_TOO_MANY_ROWS, __FILE__, __LINE__);
    }
}

std::string CDBConnectionBase::query(const char* format, ...) throw (CDBException)
{
    DBTable db_table;
    int excepted = 0;
    size_t sql_size = _sql_size;
    std::string result;
    utils::ScopedArray<char> sql(new char[sql_size]);
    va_list ap;

    while (true)
    {
        va_start(ap, format);
        excepted = vsnprintf(sql.get(), sql_size, format, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (excepted > -1 && excepted < (int)sql_size)
            break;

        /* Else try again with more space. */
        if (excepted > -1)    /* glibc 2.1 */
            sql_size = (size_t)excepted + 1; /* precisely what is needed */
        else           /* glibc 2.0 */
            sql_size *= 2;  /* twice the old size */

        sql.reset(new char[sql_size]);
    }

    do_query(db_table, sql.get(), excepted);
    if (db_table.size() > 1)
    {
        throw CDBException(sql.get(), utils::StringFormatter("too many rows: %d", (int)db_table.size()).c_str(),
                DB_ERROR_TOO_MANY_ROWS, __FILE__, __LINE__);
    }
    else if (1 == db_table.size())
    {
        const DBRow& db_row = db_table.front();
        if (1 == db_row.size())
        {
            result = db_row.front();
        }
        else if (db_row.size() > 1)
        {
            throw CDBException(sql.get(), utils::StringFormatter("too many cols: %d", (int)db_row.size()).c_str(),
                    DB_ERROR_TOO_MANY_COLS, __FILE__, __LINE__);
        }
    }

    return result;
}

void CDBConnectionBase::ping() throw (CDBException)
{
    THROW_DB_EXCEPTION(NULL, "not supported", DB_NOT_SUPPORTED);
}

void CDBConnectionBase::commit() throw (CDBException)
{
    THROW_DB_EXCEPTION(NULL, "not supported", DB_NOT_SUPPORTED);
}

void CDBConnectionBase::rollback() throw (CDBException)
{
    THROW_DB_EXCEPTION(NULL, "not supported", DB_NOT_SUPPORTED);
}

void CDBConnectionBase::enable_autocommit(bool enabled) throw (CDBException)
{
    THROW_DB_EXCEPTION(NULL, "not supported", DB_NOT_SUPPORTED);
}

bool CDBConnectionBase::is_established() const
{
    return _is_established;
}

SYS_NAMESPACE_END
