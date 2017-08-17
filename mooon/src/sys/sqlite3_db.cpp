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
#include "sys/sqlite3_db.h"
#include "utils/scoped_ptr.h"
#include "utils/string_formatter.h"
#include "utils/string_utils.h"
#include <stdarg.h>
#include <strings.h>

#if MOOON_HAVE_SQLITE3==1
#include <sqlite3/sqlite3.h>
SYS_NAMESPACE_BEGIN

// 将自己注释到CObjectFactdory中
REGISTER_OBJECT_CREATOR("sqlite3_connection", CSQLite3Connection)

CSQLite3Connection::CSQLite3Connection(size_t sql_max)
    : CDBConnectionBase(sql_max), _sqlite(NULL)
{
}

CSQLite3Connection::~CSQLite3Connection()
{
    close(); // 不要在父类的析构中调用虚拟函数
}

void CSQLite3Connection::open() throw (CDBException)
{
    sqlite3* sqlite = NULL;

    if (_db_name.empty())
    {
            throw CDBException(NULL,
                               "db name of sqlite3 not set", -1,
                               __FILE__, __LINE__);
    }
    else
    {
        _id = utils::CStringUtils::format_string("sqlite3://%s", _db_name.c_str());

        if (sqlite3_open(_db_name.c_str(), &sqlite) != SQLITE_OK)
        {
            throw CDBException(NULL,
                               sqlite3_errmsg(sqlite), sqlite3_errcode(sqlite),
                               __FILE__, __LINE__);
        }

        _sqlite = sqlite;
    }
}

void CSQLite3Connection::close() throw ()
{
    _is_established = false;
    
    if (_sqlite != NULL)
    {
        sqlite3* sqlite = static_cast<sqlite3*>(_sqlite);

        sqlite3_close(sqlite);
        _sqlite = NULL;
    }
}

void CSQLite3Connection::reopen() throw (CDBException)
{
    // 先关闭释放资源，才能再建立
    close();
    open();
}

uint64_t CSQLite3Connection::update(const char* format, ...) throw (CDBException)
{
    MOOON_ASSERT(_sqlite != NULL);

    sqlite3* sqlite = static_cast<sqlite3*>(_sqlite);
    char *errmsg = NULL;
    int excepted = 0; // sql_length
    int ret = SQLITE_OK;
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

    ret = sqlite3_exec(sqlite, sql.get(), 0, 0, &errmsg);
    if (ret != SQLITE_OK)
    {
        std::string errmsg_ = errmsg;
        sqlite3_free(errmsg);

        throw CDBException(sql.get(), utils::StringFormatter("sql[%s] error: %s", sql.get(), errmsg_.c_str()).c_str(),
                ret, __FILE__, __LINE__);
    }

    return static_cast<uint64_t>(ret);
}

std::string CSQLite3Connection::str() throw ()
{
    return _id;
}

void CSQLite3Connection::do_query(DBTable& db_table, const char* sql, int sql_length) throw (CDBException)
{
    MOOON_ASSERT(_sqlite != NULL);

    sqlite3* sqlite = static_cast<sqlite3*>(_sqlite);
    char *errmsg = NULL;
    char **table = NULL;
    int num_rows = 0;
    int num_cols = 0;

    int ret = sqlite3_get_table(
            sqlite
          , sql
          , &table
          , &num_rows
          , &num_cols
          , &errmsg);
    if (ret != SQLITE_OK)
    {
        std::string errmsg_ = errmsg;

        sqlite3_free(errmsg);
        throw CDBException(sql,
                utils::StringFormatter("sql[%s] error: %s", sql, errmsg_.c_str()).c_str(),
                ret, __FILE__, __LINE__);
    }
    else
    {
        for (int i=0; i<num_rows; ++i)
        {
            int index = (i + 1) * num_cols;

            DBRow db_row;
            for (int field_no=0; field_no<num_cols; ++field_no)
            {
                const char* field_value = table[index + field_no];

                if (NULL == field_value)
                    db_row.push_back(_null_value);
                else
                    db_row.push_back(field_value);
            }

            db_table.push_back(db_row);
        }

        sqlite3_free_table(table);
    }
}

SYS_NAMESPACE_END
#endif // MOOON_HAVE_SQLITE3
