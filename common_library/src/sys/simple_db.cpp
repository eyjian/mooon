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
#include "util/scoped_ptr.h"
#include "util/string_formatter.h"
#include "util/string_util.h"
//#include <my_global.h> // 有些版本的MySQL可能需要包含此头文件
//#include <my_sys.h>    // 有些版本的MySQL可能需要包含此头文件
#include <mysql/errmsg.h> // CR_SERVER_GONE_ERROR
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h> // ER_QUERY_INTERRUPTED
#include <stdarg.h>
#include <strings.h>
#include <sqlite3/sqlite3.h>
SYS_NAMESPACE_BEGIN

/**
 * 不同DB的通用操作
 */
class CDBConnectionBase: public DBConnection
{
public:
    CDBConnectionBase(size_t sql_max);
    
private:
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

private:
    virtual void _do_query(DBTable& db_table, const char* format, va_list& args, const char* sql, int bytes_printed) throw (CDBException) = 0;

private:
    void do_query(DBTable& db_table, const char* format, va_list& args) throw (CDBException);

protected:
    const size_t _sql_max; // 支持的最大SQL语句长度，单位为字节数，不含结尾符
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

////////////////////////////////////////////////////////////////////////////////

/**
 * MySQL版本的DB连接
 */
class CMySQLConnection: public CDBConnectionBase
{
public:
    CMySQLConnection(size_t sql_max);
    ~CMySQLConnection();

private:
    virtual void open() throw (CDBException);
    virtual void close() throw ();
    virtual void reopen() throw (CDBException);

    virtual int update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));
    virtual std::string str() throw ();

private:
    virtual void _do_query(DBTable& db_table, const char* format, va_list& args, const char* sql, int bytes_printed) throw (CDBException);

private:
    void do_open() throw (CDBException);

private:
    MYSQL* _mysql_handler; // MySQL句柄
};

////////////////////////////////////////////////////////////////////////////////

/**
 * SQLite3版本的DB连接
 */
class CSQLite3Connection: public CDBConnectionBase
{
public:
    CSQLite3Connection(size_t sql_max);
    ~CSQLite3Connection();

private:
    virtual void open() throw (CDBException);
    virtual void close() throw ();
    virtual void reopen() throw (CDBException);

    virtual int update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));
    virtual std::string str() throw ();

private:
    virtual void _do_query(DBTable& db_table, const char* format, va_list& args, const char* sql, int bytes_printed) throw (CDBException);

private:
    void do_open() throw (CDBException);

private:
    sqlite3* _sqlite;
};


////////////////////////////////////////////////////////////////////////////////
DBConnection* DBConnection::create_connection(const std::string& db_type_name, size_t sql_max)
{
    DBConnection* db_connection = NULL;

    // MySQL，不区分大小写
    if (0 == strcasecmp(db_type_name.c_str(), "mysql"))
    {
        db_connection = new CMySQLConnection(sql_max);
    }
    else if (0 == strcasecmp(db_type_name.c_str(), "sqlite3"))
    {
        db_connection = new CSQLite3Connection(sql_max);
    }

    return db_connection;
}

void DBConnection::destroy_connection(DBConnection* db_connection)
{
    delete db_connection;
}

bool DBConnection::is_disconnected_exception(CDBException& db_error)
{
    int errcode = db_error.errcode();

    // ER_QUERY_INTERRUPTED：比如mysqld进程挂了
    // CR_SERVER_GONE_ERROR：比如客户端将连接close了
    return (ER_QUERY_INTERRUPTED == errcode) || // Query execution was interrupted
           (CR_SERVER_GONE_ERROR == errcode);   // MySQL server has gone away
}

////////////////////////////////////////////////////////////////////////////////
CDBConnectionBase::CDBConnectionBase(size_t sql_max)
    : _sql_max(sql_max), _is_established(false),
      _db_port(3306), _auto_reconnect(false), _timeout_seconds(10), _null_value("$NULL$")
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

void CDBConnectionBase::enable_auto_reconnect()
{
    _auto_reconnect = true;    
}

void CDBConnectionBase::set_timeout_seconds(int timeout_seconds)
{
    _timeout_seconds = timeout_seconds;    
}

void CDBConnectionBase::set_null_value(const std::string& null_value)
{
    _null_value = null_value;
}
    
void CDBConnectionBase::query(DBTable& db_table, const char* format, ...) throw (CDBException)
{
    va_list args;
    va_start(args, format);

    try
    {
        do_query(db_table, format, args);
        va_end(args);
    }
    catch (...)
    {
        va_end(args);
        throw;
    }
}

void CDBConnectionBase::query(DBRow& db_row, const char* format, ...) throw (CDBException)
{
    DBTable db_table;
    va_list args;
    va_start(args, format);

    try
    {
        do_query(db_table, format, args);

        if (1 == db_table.size())
        {
            db_row = db_table.front();
        }
        else if (db_table.size() > 1)
        {
            throw CDBException(NULL, utils::StringFormatter("too many rows: %d", (int)db_table.size()).c_str(),
                    DB_ERROR_TOO_MANY_ROWS, __FILE__, __LINE__);
        }

        va_end(args);
    }
    catch (...)
    {
        va_end(args);
        throw;
    }
}

std::string CDBConnectionBase::query(const char* format, ...) throw (CDBException)
{
    DBTable db_table;
    std::string result;
    va_list args;
    va_start(args, format);

    try
    {
        do_query(db_table, format, args);

        if (db_table.size() > 1)
        {
            throw CDBException(NULL, utils::StringFormatter("too many rows: %d", (int)db_table.size()).c_str(),
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
                throw CDBException(NULL, utils::StringFormatter("too many cols: %d", (int)db_row.size()).c_str(),
                        DB_ERROR_TOO_MANY_COLS, __FILE__, __LINE__);
            }
        }

        va_end(args);
    }
    catch (...)
    {
        va_end(args);
        throw;
    }

    return result;
}

void CDBConnectionBase::do_query(DBTable& db_table, const char* format, va_list& args) throw (CDBException)
{
    utils::ScopedArray<char> sql(new char[_sql_max + 1]);

    // 拼成SQL语句
    int bytes_printed = vsnprintf(sql.get(), _sql_max + 1, format, args);
    if (bytes_printed >= ((int)_sql_max + 1))
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] too long: %d over %d", sql.get(), bytes_printed, (int)_sql_max).c_str(),
                -1, __FILE__, __LINE__);
    }

    _do_query(db_table, format, args, sql.get(), bytes_printed);
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

void CMySQLConnection::open() throw (CDBException)
{    
    _id = utils::CStringUtil::format_string("mysql://%s@%s:%d", _db_name.c_str(), _db_ip.c_str(), _db_port);
    do_open();
}

void CMySQLConnection::close() throw ()
{
    if (_mysql_handler != NULL)
    {
        _is_established = false;
        mysql_close(_mysql_handler);
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
    va_list args;
    va_start(args, format);
    utils::ScopedArray<char> sql(new char[_sql_max + 1]);

    // 拼成SQL语句
    int bytes_printed = vsnprintf(sql.get(), _sql_max + 1, format, args);
    va_end(args);

    if (bytes_printed >= ((int)_sql_max + 1))
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] too long: %d over %d", sql.get(), bytes_printed, (int)_sql_max).c_str(),
                -1, __FILE__, __LINE__);
    }

    // 如果查询成功，返回0。如果出现错误，返回非0值
    if (mysql_real_query(_mysql_handler, sql.get(), (unsigned long)bytes_printed) != 0)
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] error: %s", sql.get(), mysql_error(_mysql_handler)).c_str(),
                mysql_errno(_mysql_handler), __FILE__, __LINE__);
    }

    return static_cast<int>(mysql_affected_rows(_mysql_handler));
}

std::string CMySQLConnection::str() throw ()
{
    return _id;
}

void CMySQLConnection::_do_query(DBTable& db_table, const char* format, va_list& args, const char* sql, int bytes_printed) throw (CDBException)
{
    // 如果查询成功，返回0。如果出现错误，返回非0值
    if (mysql_real_query(_mysql_handler, sql, (unsigned long)bytes_printed) != 0)
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] error: %s", sql, mysql_error(_mysql_handler)).c_str(),
                mysql_errno(_mysql_handler), __FILE__, __LINE__);
    }

    // 取结果集
    MYSQL_RES* result_set = mysql_store_result(_mysql_handler);
    if (NULL == result_set)
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] error: %s", sql, mysql_error(_mysql_handler)).c_str(),
                mysql_errno(_mysql_handler), __FILE__, __LINE__);
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
    // 低版本不支持MYSQL_OPT_RECONNECT
    // 指示是否自动重连接
    my_bool reconnect = _auto_reconnect? 1: 0;

    // 分配或初始化与mysql_real_connect()相适应的MYSQL对象。如果mysql是NULL指针，该函数将分配、初始化、并返
    // 回新对象。否则，将初始化对象，并返回对象的地址。如果mysql_init()分配了新的对象，当调用mysql_close()来关闭
    // 连接时，将释放该对象。
    _mysql_handler = mysql_init(NULL);

    // 设置超时时长
    mysql_options(_mysql_handler, MYSQL_OPT_CONNECT_TIMEOUT, reinterpret_cast<char*>(&_timeout_seconds));

    // 设置自动重连接
    mysql_options(_mysql_handler, MYSQL_OPT_RECONNECT, &reconnect);

    if (!_charset.empty())
    {
        // 设置字符集
        mysql_options(_mysql_handler, MYSQL_SET_CHARSET_NAME, _charset.c_str());
    }

    try
    {
        if (NULL == mysql_real_connect(_mysql_handler,
                                       _db_ip.c_str(), _db_user.c_str(), _db_password.c_str(),
                                       _db_name.c_str(), _db_port, NULL, 0))
        {
            throw CDBException(NULL,
                               mysql_error(_mysql_handler), mysql_errno(_mysql_handler),
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

////////////////////////////////////////////////////////////////////////////////
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
    if (_db_name.empty())
    {
            throw CDBException(NULL,
                               "db name of sqlite3 not set", -1,
                               __FILE__, __LINE__);
    }
    else
    {
        _id = utils::CStringUtil::format_string("sqlite3://%s", _db_name.c_str());

        if (sqlite3_open(_db_name.c_str(), &_sqlite) != SQLITE_OK)
        {
            throw CDBException(NULL,
                               sqlite3_errmsg(_sqlite), sqlite3_errcode(_sqlite),
                               __FILE__, __LINE__);
        }
    }
}

void CSQLite3Connection::close() throw ()
{
    _is_established = false;
    
    if (_sqlite != NULL)
    {
        sqlite3_close(_sqlite);
        _sqlite = NULL;
    }
}

void CSQLite3Connection::reopen() throw (CDBException)
{
    // 先关闭释放资源，才能再建立
    close();
    open();
}

int CSQLite3Connection::update(const char* format, ...) throw (CDBException)
{
    va_list args;
    va_start(args, format);
    utils::ScopedArray<char> sql(new char[_sql_max + 1]);

    // 拼成SQL语句
    int bytes_printed = vsnprintf(sql.get(), _sql_max + 1, format, args);
    va_end(args);

    if (bytes_printed >= ((int)_sql_max + 1))
    {
        throw CDBException(NULL, utils::StringFormatter("sql[%s] too long: %d over %d", sql.get(), bytes_printed, (int)_sql_max).c_str(),
                -1, __FILE__, __LINE__);
    }
    
    int ret = SQLITE_OK;
    char *errmsg = NULL;

    ret = sqlite3_exec(_sqlite, sql.get(), 0, 0, &errmsg);
    if (ret != SQLITE_OK)
    {
        std::string errmsg_ = errmsg;
        sqlite3_free(errmsg);

        throw CDBException(NULL, utils::StringFormatter("sql[%s] error: %s", sql.get(), errmsg_.c_str()).c_str(),
                -1, __FILE__, __LINE__);
    }

    return ret;
}

std::string CSQLite3Connection::str() throw ()
{
    return _id;
}

void CSQLite3Connection::_do_query(DBTable& db_table, const char* format, va_list& args, const char* sql, int bytes_printed) throw (CDBException)
{
    int num_rows = 0;
    int num_cols = 0;
    char *errmsg = NULL;
    char **table = NULL;

    int ret = sqlite3_get_table(
            _sqlite
          , sql
          , &table
          , &num_rows
          , &num_cols
          , &errmsg);
    if (ret != SQLITE_OK)
    {
        std::string errmsg_ = errmsg;

        sqlite3_free(errmsg);
        throw CDBException(NULL,
                utils::StringFormatter("sql[%s] error: %s", sql, errmsg_.c_str()).c_str(),
                -1, __FILE__, __LINE__);
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
