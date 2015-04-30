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
 * 代码采用商业友好的Apache协议，可任意修改和分发，但请保留版权说明文字。
 * 如遇到的问题，请发送到上述邮箱，以便及时修复。谢谢合作，共创开源！ 
 *
 * 数据库操作出错时，均要求以CDBException异常的方式处理
 */
//#include <my_global.h> // 有些版本的MySQL可能需要包含此头文件
//#include <my_sys.h>    // 有些版本的MySQL可能需要包含此头文件
//#include <mysql.h>
#include <mysql/mysql.h>
#include "mysql_connection.h"
#include "util/string_util.h"
LIBPLUGIN_NAMESPACE_BEGIN

inline MYSQL_RES* get_resultset(void* resultset)
{
    return (MYSQL_RES*)resultset;
}

inline MYSQL* get_mysql_handler(void* mysql_handler)
{
    return (MYSQL*)mysql_handler;
}

//////////////////////////////////////////////////////////////////////////
// CMySQLRow

CMySQLRow::CMySQLRow(char** field_array, uint16_t filed_number)
    :_field_array(field_array)
    ,_filed_number(filed_number)
{
}

const char* CMySQLRow::get_field_value(uint16_t index) const
{
    return (index < _filed_number)? _field_array[index]: NULL;
}

const char* CMySQLRow::get_field_value(const char* filed_name) const
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CMySQLRecordset

CMySQLRecordset::CMySQLRecordset(void* resultset)
    :_resultset(resultset)
{
}

CMySQLRecordset::~CMySQLRecordset()
{
    if (_resultset != NULL)
    {
        mysql_free_result(get_resultset(_resultset));
        _resultset = NULL;
    }
}

size_t CMySQLRecordset::get_row_number() const
{
    return (size_t)mysql_num_rows(get_resultset(_resultset));
}

uint16_t CMySQLRecordset::get_field_number() const
{
    return (size_t)mysql_num_fields(get_resultset(_resultset));
}

bool CMySQLRecordset::is_empty() const
{
    return 0 == get_row_number();
}

CMySQLRow* CMySQLRecordset::get_next_recordrow() const
{
    MYSQL_ROW recordrow = mysql_fetch_row(get_resultset(_resultset));
    return (NULL == recordrow)? NULL: new CMySQLRow((char**)recordrow, get_field_number());
}

void CMySQLRecordset::free_recordrow(sys::IRecordrow* recordrow)
{
    delete (CMySQLRow*)recordrow;
}

//////////////////////////////////////////////////////////////////////////
// CMySQLConnection

CMySQLConnection::CMySQLConnection()
    :_is_established(false)
    ,_mysql_handler(NULL)
{
}

CMySQLConnection::~CMySQLConnection()
{
    close();
}

void CMySQLConnection::open(const char* db_ip
                          , uint16_t db_port
                          , const char* db_name
                          , const char* db_user
                          , const char* db_password)
{
    my_bool auto_reconnect = 1; // 设置自动重连接

    // 分配或初始化与mysql_real_connect()相适应的MYSQL对象。如果mysql是NULL指针，该函数将分配、初始化、并返
    // 回新对象。否则，将初始化对象，并返回对象的地址。如果mysql_init()分配了新的对象，当调用mysql_close()来关闭
    // 连接时，将释放该对象。
    _mysql_handler = mysql_init(NULL);
    
	mysql_options(get_mysql_handler(_mysql_handler)
                , MYSQL_OPT_RECONNECT, &auto_reconnect);

    try
    {    
        if (NULL == mysql_real_connect(
            get_mysql_handler(_mysql_handler)
          , db_ip, db_user, db_password, db_name, db_port, NULL, 0))
        {
            throw sys::CDBException(
                NULL
              , mysql_error(get_mysql_handler(_mysql_handler))
              , mysql_errno(get_mysql_handler(_mysql_handler))
              , __FILE__, __LINE__);    
        }
        
        _is_established = true;
    }
    catch (sys::CDBException& ex)
    {
        close();
        throw;
    }    
}

void CMySQLConnection::close()
{    
    if (_mysql_handler != NULL)
    {
        _is_established = false;
        mysql_close(get_mysql_handler(_mysql_handler));
        _mysql_handler = NULL;        
    }
}

void CMySQLConnection::enable_autocommit(bool enabled)
{
    mysql_autocommit(get_mysql_handler(_mysql_handler), enabled? 1: 0);
}

bool CMySQLConnection::is_established() const
{
    return _is_established;
}

CMySQLRecordset* CMySQLConnection::query(const char* format, va_list& args)
{
    char sql[SQL_MAX];    
    int sql_length = utils::CStringUtils::fix_vsnprintf(sql, sizeof(sql), format, args);

    // 如果查询成功，返回0。如果出现错误，返回非0值
    if (mysql_real_query(get_mysql_handler(_mysql_handler), sql, (unsigned long)sql_length) != 0)
    {
        throw sys::CDBException(
            sql
          , mysql_error(get_mysql_handler(_mysql_handler))
          , mysql_errno(get_mysql_handler(_mysql_handler))
          , __FILE__, __LINE__);
    }
    
    bool is_stored = true;
    MYSQL_RES* resultset = is_stored
                         ? mysql_store_result(get_mysql_handler(_mysql_handler))
                         : mysql_use_result(get_mysql_handler(_mysql_handler));
    if (NULL == resultset)
    {
        throw sys::CDBException(
            sql
          , mysql_error(get_mysql_handler(_mysql_handler))
          , mysql_errno(get_mysql_handler(_mysql_handler))
          , __FILE__, __LINE__);
    }
    
    return new CMySQLRecordset(resultset);
}

CMySQLRecordset* CMySQLConnection::query(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);

    return query(format, args);
}

int CMySQLConnection::query(sys::DbTable* table, const char* format, va_list& args)
{
    CMySQLRecordset* recordset = query(format, args);
    size_t num_rows = recordset->get_row_number();
    size_t num_cols = recordset->get_field_number();
    table->resize(num_rows);

    int row_index = 0;
    while (true)
    {
        CMySQLRow* row = recordset->get_next_recordrow();
        if (NULL == row)
        {
            break;
        }

        sys::DbFields& fields = (*table)[row_index++];
        fields.resize(num_cols);
        for (int i=0; i<static_cast<int>(num_cols); ++i)
        {
            const char* value = row->get_field_value(i);
            fields[i] = value;
        }
    }

    free_recordset(recordset);
    return static_cast<int>(table->size());
}

int CMySQLConnection::query(sys::DbTable* table, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);

    return query(table, format, args);
}

int CMySQLConnection::get_field_value(std::string* value, const char* format, va_list& args)
{
    sys::DbTable table;
    int num_rows = query(&table, format, args);
    sys::DbFields& db_fields = table[0];
    *value = db_fields[0];

    return num_rows;
}

int CMySQLConnection::get_field_value(std::string* value, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);

    return get_field_value(value, format, args);
}

int CMySQLConnection::get_fields_value(sys::DbFields *values, const char* format, va_list& args)
{
    sys::DbTable table;
    int num_rows = query(&table, format, args);

    std::copy(table[0].begin(), table[0].end(), std::back_inserter(*values));
    return num_rows;
}

int CMySQLConnection::get_fields_value(sys::DbFields *values, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);

    return get_fields_value(values, format, args);
}

void CMySQLConnection::free_recordset(sys::IRecordset* recordset)
{
    delete (CMySQLRecordset*)recordset;
}

size_t CMySQLConnection::update(const char* format, va_list& args)
{
    char sql[SQL_MAX];    
    int sql_length = utils::CStringUtils::fix_vsnprintf(sql, sizeof(sql), format, args);

    // 如果查询成功，返回0。如果出现错误，返回非0值
    if (mysql_real_query(get_mysql_handler(_mysql_handler), sql, (unsigned long)sql_length) != 0)
    {
        throw sys::CDBException(
            sql
          , mysql_error(get_mysql_handler(_mysql_handler))
          , mysql_errno(get_mysql_handler(_mysql_handler))
          , __FILE__, __LINE__);
    }
    
    size_t num_rows_affected = static_cast<size_t>(mysql_affected_rows(get_mysql_handler(_mysql_handler)));
    return num_rows_affected;
}

size_t CMySQLConnection::update(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);
    
    size_t num_rows_affected = update(format, args);
    return num_rows_affected;
}

LIBPLUGIN_NAMESPACE_END
