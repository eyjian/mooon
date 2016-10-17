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
#include "sys/mysql_db.h"
#include "utils/scoped_ptr.h"
#include "utils/string_formatter.h"
#include "utils/string_utils.h"

#if HAVE_MYSQL==1
//#include <my_global.h> // 有些版本的MySQL可能需要包含此头文件
//#include <my_sys.h>    // 有些版本的MySQL可能需要包含此头文件
#include <mysql/errmsg.h> // CR_SERVER_GONE_ERROR
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h> // ER_QUERY_INTERRUPTED
#include <stdarg.h>
#include <strings.h>
SYS_NAMESPACE_BEGIN

// 将自己注释到CObjectFactdory中
REGISTER_OBJECT_CREATOR("mysql_connection", CMySQLConnection)

// 用来标识是否成功调用过library_init
static bool sg_library_initialized = false;

bool CMySQLConnection::library_init(int argc, char **argv, char **groups)
{
    if (!sg_library_initialized)
    {
        // In a nonmulti-threaded environment, the call to mysql_library_init() may be omitted,
        // because mysql_init() will invoke it automatically as necessary.
        // However, mysql_library_init() is not thread-safe in a multi-threaded environment,
        // and thus neither is mysql_init(), which calls mysql_library_init().
        // You must either call mysql_library_init() prior to spawning any threads
        // or else use a mutex to protect the call, whether you invoke mysql_library_init() or indirectly through mysql_init().
        sg_library_initialized = (0 == mysql_library_init(argc, argv, groups));
    }

    return sg_library_initialized;
}

void CMySQLConnection::library_end()
{
    if (sg_library_initialized)
    {
        // This function finalizes the MySQL library. Call it when you are done using the library
        mysql_library_end();
        sg_library_initialized = false;
    }
}

bool CMySQLConnection::is_duplicate(int errcode)
{
    return ER_DUP_ENTRY == errcode; // 1062
}

void CMySQLConnection::escape_string(const std::string& str, std::string* escaped_str)
{
    escaped_str->resize(str.size()*2+1, '\0');
    int escaped_string_length = mysql_escape_string(const_cast<char*>(escaped_str->data()), str.c_str(), str.length());
    escaped_str->resize(escaped_string_length);
}

////////////////////////////////////////////////////////////////////////////////
CMySQLConnection::CMySQLConnection(size_t sql_max)
    : CDBConnectionBase(sql_max), _mysql_handler(NULL)
{
}

CMySQLConnection::~CMySQLConnection()
{
    close(); // 不要在父类的析构中调用虚拟函数
}

bool CMySQLConnection::is_syntax_exception(int errcode) const
{
    return ER_PARSE_ERROR == errcode; // 1064
}

bool CMySQLConnection::is_duplicate_exception(int errcode) const
{
    return CMySQLConnection::is_duplicate(errcode);
}

bool CMySQLConnection::is_disconnected_exception(CDBException& db_error) const
{
    int errcode = db_error.errcode();

    // ER_QUERY_INTERRUPTED：比如mysqld进程挂了
    // CR_SERVER_GONE_ERROR：比如客户端将连接close了
    return (ER_QUERY_INTERRUPTED == errcode) || // Query execution was interrupted
           (CR_CONN_HOST_ERROR == errcode) ||   // Can't connect to MySQL server
           (CR_SERVER_GONE_ERROR == errcode);   // MySQL server has gone away
}

std::string CMySQLConnection::escape_string(const std::string& str) const
{
    MOOON_ASSERT(_mysql_handler != NULL);

    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);
    int escaped_string_length = 0;
    std::string escaped_string(str.size()*2+1, '\0');

    if (NULL == _mysql_handler)
    {
        escaped_string_length = mysql_escape_string(
            const_cast<char*>(escaped_string.data()), str.c_str(), str.length());
    }
    else
    {
        escaped_string_length = mysql_real_escape_string(
            mysql_handler, const_cast<char*>(escaped_string.data()), str.c_str(), str.length());
    }

    escaped_string.resize(escaped_string_length);
    return escaped_string;
}

void CMySQLConnection::open() throw (CDBException)
{    
    _id = utils::CStringUtils::format_string("mysql://%s@%s:%d", _db_name.c_str(), _db_ip.c_str(), _db_port);
    do_open();
}

void CMySQLConnection::close() throw ()
{
    if (_mysql_handler != NULL)
    {
        MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);

        _is_established = false;
        mysql_close(mysql_handler);
        _mysql_handler = NULL;
    }
}

void CMySQLConnection::reopen() throw (CDBException)
{
    // 先关闭释放资源，才能再建立
    close();
    do_open();
}

int CMySQLConnection::update(const char* format, ...) throw (CDBException)
{
    MOOON_ASSERT(_mysql_handler != NULL);

    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);
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

    // 如果查询成功，返回0。如果出现错误，返回非0值
    if (mysql_real_query(mysql_handler, sql.get(), (unsigned long)excepted) != 0)
    {
        throw CDBException(sql.get(), utils::StringFormatter("sql[%s] error: %s", sql.get(), mysql_error(mysql_handler)).c_str(),
                mysql_errno(mysql_handler), __FILE__, __LINE__);
    }

    return static_cast<int>(mysql_affected_rows(mysql_handler));
}

std::string CMySQLConnection::str() throw ()
{
    return _id;
}

void CMySQLConnection::ping() throw (CDBException)
{
    MOOON_ASSERT(_mysql_handler != NULL);
    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);

    if (mysql_ping(mysql_handler) != 0)
        THROW_DB_EXCEPTION(NULL, mysql_error(mysql_handler), mysql_errno(mysql_handler));
}

void CMySQLConnection::commit() throw (CDBException)
{
    MOOON_ASSERT(_mysql_handler != NULL);
    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);

    if (mysql_commit(mysql_handler) != 0)
        THROW_DB_EXCEPTION(NULL, mysql_error(mysql_handler), mysql_errno(mysql_handler));
}

void CMySQLConnection::rollback() throw (CDBException)
{
    MOOON_ASSERT(_mysql_handler != NULL);
    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);

    if (mysql_rollback(mysql_handler) != 0)
        THROW_DB_EXCEPTION(NULL, mysql_error(mysql_handler), mysql_errno(mysql_handler));
}

void CMySQLConnection::enable_autocommit(bool enabled) throw (CDBException)
{
    MOOON_ASSERT(_mysql_handler != NULL);
    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);

    my_bool auto_mode = enabled? 1: 0;
    if (mysql_autocommit(mysql_handler, auto_mode) != 0)
        THROW_DB_EXCEPTION(NULL, mysql_error(mysql_handler), mysql_errno(mysql_handler));
}

void CMySQLConnection::do_query(DBTable& db_table, const char* sql, int sql_length) throw (CDBException)
{
    MOOON_ASSERT(_mysql_handler != NULL);
    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);

    // 如果查询成功，返回0。如果出现错误，返回非0值
    if (mysql_real_query(mysql_handler, sql, (unsigned long)sql_length) != 0)
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] error: %s", sql, mysql_error(mysql_handler)).c_str(),
                mysql_errno(mysql_handler), __FILE__, __LINE__);
    }

    // 取结果集
    MYSQL_RES* result_set = mysql_store_result(mysql_handler);
    if (NULL == result_set)
    {
        throw CDBException(sql, utils::StringFormatter("sql[%s] error: %s", sql, mysql_error(mysql_handler)).c_str(),
                mysql_errno(mysql_handler), __FILE__, __LINE__);
    }
    else
    {
        // 取得字段个数
        int num_fields = mysql_num_fields(result_set);

        while (true)
        {
            MYSQL_ROW row = mysql_fetch_row(result_set);
            if (NULL == row)
            {
                break;
            }

            DBRow db_row;
            for (int i = 0; i < num_fields; ++i)
            {
                const char* field_value = row[i];

                if (NULL == field_value)
                    db_row.push_back(_null_value);
                else
                    db_row.push_back(field_value);
            }

            db_table.push_back(db_row);
        }

        mysql_free_result(result_set);
    }
}

void CMySQLConnection::do_open() throw (CDBException)
{
    MOOON_ASSERT(NULL == _mysql_handler);

    // 低版本不支持MYSQL_OPT_RECONNECT
    // 指示是否自动重连接
    my_bool reconnect = _auto_reconnect? 1: 0;

    // 分配或初始化与mysql_real_connect()相适应的MYSQL对象。如果mysql是NULL指针，该函数将分配、初始化、并返
    // 回新对象。否则，将初始化对象，并返回对象的地址。如果mysql_init()分配了新的对象，当调用mysql_close()来关闭
    // 连接时，将释放该对象。
    _mysql_handler = mysql_init(NULL);
    MYSQL* mysql_handler = static_cast<MYSQL*>(_mysql_handler);
    MOOON_ASSERT(mysql_handler != NULL);

    // 设置超时时长
    mysql_options(mysql_handler, MYSQL_OPT_CONNECT_TIMEOUT, reinterpret_cast<char*>(&_timeout_seconds));

    // 设置自动重连接
    mysql_options(mysql_handler, MYSQL_OPT_RECONNECT, &reconnect);

    if (!_charset.empty())
    {
        // 设置字符集
        mysql_options(mysql_handler, MYSQL_SET_CHARSET_NAME, _charset.c_str());
    }

    try
    {
        if (NULL == mysql_real_connect(mysql_handler,
                                       _db_ip.c_str(), _db_user.c_str(), _db_password.c_str(),
                                       _db_name.c_str(), _db_port, NULL, 0))
        {
            throw CDBException(NULL,
                               mysql_error(mysql_handler), mysql_errno(mysql_handler),
                               __FILE__, __LINE__);
        }

        _is_established = true;
    }
    catch (CDBException&)
    {
        close();
        throw;
    }
}

SYS_NAMESPACE_END
#endif // HAVE_MYSQL
