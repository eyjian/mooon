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
#ifndef MOOON_SYS_DB_H
#define MOOON_SYS_DB_H
#include "mooon/sys/db_exception.h"
#include <vector>
#define SQL_MAX 4096 /** 一条SQL语句允许的最大长度 */
SYS_NAMESPACE_BEGIN

typedef std::vector<std::string> DbFields; // 用来存储表的一行的所有字段值
typedef std::vector<DbFields > DbTable;    // 用来存储表的多行数据

/***
  * 记录行接口
  */
class IRecordrow
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IRecordrow() {}

    /***
      * 通过字段编号取得字段的值
      */
    virtual const char* get_field_value(uint16_t index) const = 0;
};

/***
  * 记录集接口
  */
class IRecordset
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IRecordset() {}

    /***
      * 得到记录集的行数
      * 对于MySQL，如果query时，参数is_stored为false，则该函数不能返回正确的值，
      * 所以应当只有在is_stored为true，才使用该函数
      */
    virtual size_t get_row_number() const = 0;

    /***
      * 得到字段个数
      */
    virtual uint16_t get_field_number() const = 0;

    /***
      * 判断记录集是否为空
      */
    virtual bool is_empty() const = 0;

    /***
      * 检索结果集的下一行
      * @return: 如果没有要检索的行返回NULL，否则返回指向记录行的指针，这时必须调用release_recordrow，否则有内存泄漏
      */
    virtual IRecordrow* get_next_recordrow() const = 0;

    /***
      * 释放get_next_recordrow得到的记录行
      */
    virtual void free_recordrow(IRecordrow* recordrow) = 0;
};

/***
  * 一般性的数据库连接接口
  * 使用引用计数管理生命周期
  */
class IDBConnection
{
public:    
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IDBConnection() {}

    /** 对引用计数值增一 */
    virtual void inc_refcount() = 0;

    /***
      * 对引用计数值减一
      * 如果减去之后，引用计数值为0，则执行自删除
      * @return: 如果减去之后引用计数为0，则返回true，这个时候对象自身也被删除了
      */
    virtual bool dec_refcount() = 0;

    /** 是否允许自动提交 */
    virtual void enable_autocommit(bool enabled) = 0;  
    
    /***
      * 用来判断数据库连接是否正建立着
      */
    virtual bool is_established() const = 0;

    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @return: 如成功返回记录集的指针，这时必须调用release_recordset，否则有内存泄漏
      * @exception: 如出错抛出CDBException异常
      */
    virtual IRecordset* query(const char* format, ...) __attribute__((format(printf, 2, 3))) = 0;
    virtual int query(DbTable* table, const char* format, ...) __attribute__((format(printf, 3, 4))) = 0;

    /***
     * 取单个字段第一行的值
     * @value 用来存储单个字段第一行的值
     * @return 返回符合条件的行数
     */
    virtual int get_field_value(std::string* value, const char* format, va_list& args) = 0;
    virtual int get_field_value(std::string* value, const char* format, ...) __attribute__((format(printf, 3, 4))) = 0;

    /***
     * 取得多个字段的第一行的值
     */
    virtual int get_fields_value(DbFields *values, const char* format, va_list& args) = 0;
    virtual int get_fields_value(DbFields *values, const char* format, ...) __attribute__((format(printf, 3, 4))) = 0;

    /***
      * 释放query得到的记录集
      */
    virtual void free_recordset(IRecordset* recordset) = 0;

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    virtual size_t update(const char* format, ...) __attribute__((format(printf, 2, 3))) = 0;
};

/***
  * 用于数据库连接池的数据库连接接口
  */
class IDBPoolConnection
{
public:    
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IDBPoolConnection() {}

    /** 是否允许自动提交 */
    virtual void enable_autocommit(bool enabled) = 0;  
    
    /***
      * 用来判断数据库连接是否正建立着
      */
    virtual bool is_established() const = 0;

    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @return: 如成功返回记录集的指针，这时必须调用release_recordset，否则有内存泄漏
      * @exception: 如出错抛出CDBException异常
      */
    virtual IRecordset* query(const char* format, ...) __attribute__((format(printf, 2, 3))) = 0;
    virtual int query(DbTable* table, const char* format, ...) __attribute__((format(printf, 3, 4))) = 0;
    
    /***
     * 取单个字段第一行的值
     * @value 用来存储单个字段第一行的值
     * @return 返回符合条件的行数
     */
    virtual int get_field_value(std::string* value, const char* format, va_list& args) = 0;
    virtual int get_field_value(std::string* value, const char* format, ...) __attribute__((format(printf, 3, 4))) = 0;

    /***
     * 取得多个字段的第一行的值
     */
    virtual int get_fields_value(DbFields *values, const char* format, va_list& args) = 0;
    virtual int get_fields_value(DbFields *values, const char* format, ...) __attribute__((format(printf, 3, 4))) = 0;

    /***
      * 释放query得到的记录集
      */
    virtual void free_recordset(IRecordset* recordset) = 0;

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    virtual size_t update(const char* format, ...) __attribute__((format(printf, 2, 3))) = 0;
};

/***
  * 数据库连接池接口
  */
class IDBConnectionPool
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IDBConnectionPool() {}

    /***
      * 得到全小写形式的数据库类型名，如：mysql和postgresql等
      */
    virtual const char* get_type_name() const = 0;
    
    /***
      * 线程安全函数
      * 从数据库连接池中获取一个连接
      * @return: 如果当前无可用的连接，则返回NULL，否则返回指向数据库连接的指针
      * @exception: 不会抛出任何异常
      */
    virtual IDBPoolConnection* get_connection() = 0;

    /***
      * 线程安全函数
      * 将已经获取的数据库连接放回到数据库连接池中
      * @exception: 不会抛出任何异常
      */
    virtual void put_connection(IDBPoolConnection* db_connection) = 0;

    /***
      * 创建连接池
      * 非线程安全，只能被调用一次，而且要求和destroy成对调用
      * @pool_size: 数据库连接池中的数据库连接个数
      * @db_ip: 需要连接的数据库IP地址
      * @db_port: 需要连接的数据库服务端口号
      * @db_name: 需要连接的数据库池
      * @db_user: 连接数据库用的用户名
      * @db_password: 连接数据库用的密码
      * @exception: 如出错抛出CDBException异常
      */
    virtual void create(uint16_t pool_size, const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password) = 0;

    /***
      * 销毁已经创建的数据库连接池
      * 非线程安全，只能被调用一次，而且要求和destroy成对调用
      */
    virtual void destroy() = 0;

    /***
      * 得到连接池中的连接个数
      */
    virtual uint16_t get_connection_number() const = 0;
};

/***
  * 数据库连接工厂，用于创建DBGeneralConnection类型的连接
  */
class IDBConnectionFactory
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IDBConnectionFactory() {}

    /***
      * 创建DBGeneralConnection类型的连接
      * 线程安全
      * @db_ip: 需要连接的数据库IP地址
      * @db_port: 需要连接的数据库服务端口号
      * @db_name: 需要连接的数据库池
      * @db_user: 连接数据库用的用户名
      * @db_password: 连接数据库用的密码
      * @return: 返回一个指向一般性数据库连接的指针
      * @exception: 如出错抛出CDBException异常
      */
    virtual IDBConnection* create_connection(const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password) = 0;

    /***
      * 创建数据库连接池
      * @return: 返回指向数据库连接池的指针
      * @exception: 如出错抛出CDBException异常
      */
    virtual IDBConnectionPool* create_connection_pool() = 0;

    /***
      * 销毁数据库连接池
      * @db_connection_pool: 指向需要销毁的数据库连接池的指针，
      *                      函数返回后，db_connection_pool总是被置为NULL
      */
    virtual void destroy_connection_pool(IDBConnectionPool*& db_connection_pool) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 助手类: DBConnectionPoolHelper, DBPoolConnectionHelper, RecordsetHelper和RecordrowHelper

/***
  * DBConnectionPool助手类，用于自动销毁数据库连接池
  */
class DBConnectionPoolHelper
{
public:
    DBConnectionPoolHelper(IDBConnectionFactory* db_connection_factory, IDBConnectionPool*& db_connection_pool)
        :_db_connection_factory(db_connection_factory)
        ,_db_connection_pool(db_connection_pool)
    {
    }

    /** 析构函数，自动调用destroy_connection_pool */
    ~DBConnectionPoolHelper()
    {
        if ((_db_connection_factory != NULL) && (_db_connection_pool != NULL))
        {
            _db_connection_factory->destroy_connection_pool(_db_connection_pool);
            _db_connection_pool = NULL;
        }
    }

    IDBConnectionPool* operator ->()
    {
        return _db_connection_pool;
    }

private:
    IDBConnectionFactory* _db_connection_factory;
    IDBConnectionPool*& _db_connection_pool;
};

/***
  * DBPoolConnection助手类，用于自动释放已经获取的DB连接
  */
class DBPoolConnectionHelper
{
public:
    DBPoolConnectionHelper(IDBConnectionPool* db_connection_pool, IDBPoolConnection*& db_connection)
        :_db_connection_pool(db_connection_pool)
        ,_db_connection(db_connection)
    {
    }
    
    /** 析构中将自动调用put_connection */
    ~DBPoolConnectionHelper()
    {
        if ((_db_connection_pool != NULL) && (_db_connection != NULL))
        {
            _db_connection_pool->put_connection(_db_connection);
            _db_connection = NULL;
        }
    }

    IDBPoolConnection* operator ->()
    {
        return _db_connection;
    }

private:
    IDBConnectionPool* _db_connection_pool;
    IDBPoolConnection*& _db_connection;
};

/***
  * 记录集助手类，用于自动调用free_recordset
  */
template <class DBConnectionClass>
class RecordsetHelper
{
public:
    RecordsetHelper(DBConnectionClass* db_connection, IRecordset* recordset)
        :_db_connection(db_connection)
        ,_recordset(recordset)
    {        
    }

    /** 析构中将自动调用free_recordset */
    ~RecordsetHelper()
    {
        if ((_db_connection != NULL) && (_recordset != NULL))
            _db_connection->free_recordset(_recordset);
    }

    IRecordset* operator ->()
    {
        return _recordset;
    }

private:
    DBConnectionClass* _db_connection;
    IRecordset* _recordset;
};

/***
  * 记录行助手类，用于自动调用free_recordrow
  */
class RecordrowHelper
{
public:
    RecordrowHelper(IRecordset* recordset, IRecordrow* recordrow)
        :_recordset(recordset)
        ,_recordrow(recordrow)
    {        
    }

    /** 析构中将自动调用free_recordrow */
    ~RecordrowHelper()
    {
        if ((_recordset != NULL) && (_recordrow != NULL))
            _recordset->free_recordrow(_recordrow);
    }

    IRecordrow* operator ->()
    {
        return _recordrow;
    }

private:
    IRecordset* _recordset;
    IRecordrow* _recordrow;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_DB_H
