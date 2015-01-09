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
#ifndef MOOON_PLUGIN_MYSQL_CONNECTION_H
#define MOOON_PLUGIN_MYSQL_CONNECTION_H
#include "sys/db.h"
#include "plugin/plugin.h"
#include "sys/ref_countable.h"
LIBPLUGIN_NAMESPACE_BEGIN

/***
  * MySQL行
  */
class CMySQLRow: public sys::IRecordrow
{
public:
    CMySQLRow(char** field_array, uint16_t filed_number);
        
    /***
      * 通过字段编号取得字段的值(高效率)
      */
    virtual const char* get_field_value(uint16_t index) const;

    /***
      * 通过字段名称取得字段的值(低效率)
      */
    virtual const char* get_field_value(const char* filed_name) const;

private:
    char** _field_array;
    uint16_t _filed_number;
};

/***
  * MySQL记录集
  */
class CMySQLRecordset: public sys::IRecordset
{
public:
    CMySQLRecordset(void* resultset);
    ~CMySQLRecordset();

    /***
      * 得到记录集的行数
      * 对于MySQL，如果query时，参数is_stored为false，则该函数不能返回正确的值，
      * 所以应当只有在is_stored为true，才使用该函数
      */
    virtual size_t get_row_number() const;

    /***
      * 得到字段个数
      */
    virtual uint16_t get_field_number() const;

    /***
      * 判断记录集是否为空
      */
    virtual bool is_empty() const;

    /***
      * 检索结果集的下一行
      * @return: 如果没有要检索的行返回NULL
      */
    virtual CMySQLRow* get_next_recordrow() const;

    /***
      * 释放get_next_recordrow得到的记录行
      */
    virtual void free_recordrow(sys::IRecordrow* recordrow);

private:
    void* _resultset; /** 存储MySQL结果集 */
};

/***
  * 数据库连接接口
  */
class CMySQLConnection
{
public:    
    CMySQLConnection();
    ~CMySQLConnection();
     
    void open(const char* db_ip
            , uint16_t db_port
            , const char* db_name
            , const char* db_user
            , const char* db_password);
    void close();

    /** 是否允许自动提交 */
    void enable_autocommit(bool enabled);
    
    /***
      * 用来判断数据库连接是否正建立着 
      */
    bool is_established() const;
    
    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @return: 如成功返回记录集的指针
      * @exception: 如出错抛出CDBException异常
      */
    CMySQLRecordset* query(const char* format, va_list& args);
    CMySQLRecordset* query(const char* format, ...) __attribute__((format(printf, 2, 3)));
    int query(sys::DbTable* table, const char* format, va_list& args);
    int query(sys::DbTable* table, const char* format, ...);
    
    /***
     * 取单个字段第一行的值
     * @value 用来存储单个字段第一行的值
     * @return 返回符合条件的行数
     */
    int get_field_value(std::string* value, const char* format, va_list& args);
    int get_field_value(std::string* value, const char* format, ...) __attribute__((format(printf, 3, 4)));

    /***
     * 取得多个字段的第一行的值
     */
    int get_fields_value(sys::DbFields *values, const char* format, va_list& args);
    int get_fields_value(sys::DbFields *values, const char* format, ...) __attribute__((format(printf, 3, 4)));

    /***
      * 释放query得到的记录集
      */
    void free_recordset(sys::IRecordset* recordset);

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    size_t update(const char* format, va_list& args);
    size_t update(const char* format, ...) __attribute__((format(printf, 2, 3)));

private:
    bool _is_established;   /** 是否已经和数据库建立的连接 */
    void* _mysql_handler;   /** MySQL句柄, 使用void类型是为减少头文件的依赖 */
};

class CMySQLGeneralConnection: public sys::IDBConnection
{
public:    
    CMySQLGeneralConnection()
    {
        _ref_countable = new sys::CRefCountable;
    }
    
    void open(const char* db_ip
            , uint16_t db_port
            , const char* db_name
            , const char* db_user
            , const char* db_password)
    {
        _mysql_connection.open(db_ip, db_port, db_name, db_user, db_password);
    }

    void close()
    {
        _mysql_connection.close();
    }

private:
    /** 对引用计数值增一 */
    virtual void inc_refcount()
    {
        _ref_countable->inc_refcount();
    }

    /***
      * 对引用计数值减一
      * 如果减去之后，引用计数值为0，则执行自删除
      */
    virtual bool dec_refcount()
    {
        bool deleted = _ref_countable->dec_refcount();
        if (deleted)
            delete this;

        return deleted;
    }
    
    /** 是否允许自动提交 */
    virtual void enable_autocommit(bool enabled)
    {
        _mysql_connection.enable_autocommit(enabled);
    }
    
    /***
      * 用来判断数据库连接是否正建立着 
      */
    virtual bool is_established() const
    {
        return _mysql_connection.is_established();
    }
    
    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @return: 如成功返回记录集的指针
      * @exception: 如出错抛出CDBException异常
      */
    virtual sys::IRecordset* query(const char* format, ...) __attribute__((format(printf, 2, 3)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.query(format, args);
    }
    
    virtual int query(sys::DbTable* table, const char* format, ...) __attribute__((format(printf, 3, 4)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.query(table, format, args);
    }

    /***
     * 取单个字段第一行的值
     * @value 用来存储单个字段第一行的值
     * @return 返回符合条件的行数
     */
    int get_field_value(std::string* value, const char* format, va_list& args)
    {
        return _mysql_connection.get_field_value(value, format, args);
    }

    int get_field_value(std::string* value, const char* format, ...) __attribute__((format(printf, 3, 4)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.get_field_value(value, format, args);
    }

    /***
     * 取得多个字段的第一行的值
     */
    int get_fields_value(sys::DbFields *values, const char* format, va_list& args)
    {
        return _mysql_connection.get_fields_value(values, format, args);
    }

    int get_fields_value(sys::DbFields *values, const char* format, ...) __attribute__((format(printf, 3, 4)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.get_fields_value(values, format, args);
    }

    /***
      * 释放query得到的记录集
      */
    virtual void free_recordset(sys::IRecordset* recordset)
    {
        _mysql_connection.free_recordset(recordset);
    }

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    virtual size_t update(const char* format, ...) __attribute__((format(printf, 2, 3)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);
        
        size_t num_rows_affected = _mysql_connection.update(format, args);
        return num_rows_affected;
    }

private:    
    sys::CRefCountable* _ref_countable;
    CMySQLConnection _mysql_connection;
};

class CMySQLPoolConnection: public sys::IDBPoolConnection
{
public:    
    CMySQLPoolConnection()
        :_in_pool(false)
    {        
    }
    
    bool is_in_pool() const
    {
        return _in_pool;
    }
    
    void set_in_pool(bool yes)
    {
        _in_pool = yes;
    }
    
    void open(const char* db_ip
            , uint16_t db_port
            , const char* db_name
            , const char* db_user
            , const char* db_password)
    {
        _mysql_connection.open(db_ip, db_port, db_name, db_user, db_password);
    }

    void close()
    {
        _mysql_connection.close();
    }

private:
    /** 是否允许自动提交 */
    virtual void enable_autocommit(bool enabled)
    {
        _mysql_connection.enable_autocommit(enabled);
    }
    
    /***
      * 用来判断数据库连接是否正建立着 
      */
    virtual bool is_established() const
    {
        return _mysql_connection.is_established();
    }
    
    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @return: 如成功返回记录集的指针
      * @exception: 如出错抛出CDBException异常
      */
    virtual sys::IRecordset* query(const char* format, ...) __attribute__((format(printf, 2, 3)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.query(format, args);
    }
    
    virtual int query(sys::DbTable* table, const char* format, ...) __attribute__((format(printf, 3, 4)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.query(table, format, args);
    }

    /***
     * 取单个字段第一行的值
     * @value 用来存储单个字段第一行的值
     * @return 返回符合条件的行数
     */
    int get_field_value(std::string* value, const char* format, va_list& args)
    {
        return _mysql_connection.get_field_value(value, format, args);
    }

    int get_field_value(std::string* value, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.get_field_value(value, format, args);
    }

    /***
     * 取得多个字段的第一行的值
     */
    int get_fields_value(sys::DbFields *values, const char* format, va_list& args)
    {
        return _mysql_connection.get_fields_value(values, format, args);
    }

    int get_fields_value(sys::DbFields *values, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);

        return _mysql_connection.get_fields_value(values, format, args);
    }

    /***
      * 释放query得到的记录集
      */
    virtual void free_recordset(sys::IRecordset* recordset)
    {
        _mysql_connection.free_recordset(recordset);
    }

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    virtual size_t update(const char* format, ...) __attribute__((format(printf, 2, 3)))
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vlh(args);
        
        size_t num_rows_affected = _mysql_connection.update(format, args);
        return num_rows_affected;
    }

private:
    bool _in_pool; /** 是否在连接池中 */
    CMySQLConnection _mysql_connection;
};

LIBPLUGIN_NAMESPACE_END
#endif // MOOON_PLUGIN_MYSQL_CONNECTION_H
