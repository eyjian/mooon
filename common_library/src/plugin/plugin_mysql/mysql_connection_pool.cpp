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
#include "mysql_connection_pool.h"
#include "plugin/plugin_mysql/plugin_mysql.h"
LIBPLUGIN_NAMESPACE_BEGIN

CMySQLConnectionPool::CMySQLConnectionPool()
    :_db_port(3306) // MySQL默认端口    
    ,_connect_array(NULL)
    ,_connection_queue(NULL)
{
}

const char* CMySQLConnectionPool::get_type_name() const
{
    return "mysql";
}

sys::IDBPoolConnection* CMySQLConnectionPool::get_connection()
{
    sys::LockHelper<sys::CLock> lock(_lock);
    if (_connection_queue->is_empty()) return NULL;
    
    CMySQLPoolConnection* db_connection = _connection_queue->pop_front();
    db_connection->set_in_pool(false);
    return db_connection;
}

void CMySQLConnectionPool::put_connection(sys::IDBPoolConnection* db_connection)
{
    if (db_connection != NULL)
    {
        CMySQLPoolConnection* mysql_connection = (CMySQLPoolConnection*)db_connection;
        sys::LockHelper<sys::CLock> lock(_lock);
        // 加上is_in_pool是为了防止重复放进去，降低使用难度
        if (!_connection_queue->is_full() && !mysql_connection->is_in_pool())
        {            
            _connection_queue->push_back(mysql_connection);
            mysql_connection->set_in_pool(true);
        }
    }
}

void CMySQLConnectionPool::create(uint16_t pool_size, const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password)
{
    // 保存连接数据库的信息
    _db_ip = db_ip;
    _db_port = db_port;
    _db_name = db_name;
    _db_user = db_user;
    _db_password = db_password;

    // 创建连接队列
    uint16_t db_connection_number = (0 ==pool_size)? 1: pool_size;
    _connection_queue = new util::CArrayQueue<CMySQLPoolConnection*>(db_connection_number);    
    _connect_array = new CMySQLPoolConnection[db_connection_number];

    try
    {    
        for (uint16_t i=0; i<db_connection_number; ++i)
        {
            CMySQLPoolConnection* db_connection = &_connect_array[i];
            db_connection->open(db_ip, db_port, db_name, db_user, db_password);
            _connection_queue->push_back(db_connection);
        }
    }
    catch (sys::CDBException& ex)
    {
        destroy();
        throw;
    }
}

void CMySQLConnectionPool::destroy()
{
    // 关闭所有已经建立的连接
    uint16_t db_connection_number = (uint16_t)_connection_queue->size();
    for (uint16_t i=0; i<db_connection_number; ++i)
    {
        CMySQLPoolConnection* db_connection = &_connect_array[i];
        db_connection->close();
    }
    
    // 释放连接数组
    if (_connect_array != NULL)
    {
        delete []_connect_array;
        _connect_array = NULL;
    }

    // 释放连接队列
    if (_connection_queue != NULL)
    {
        delete _connection_queue;
        _connection_queue = NULL;
    }
}

uint16_t CMySQLConnectionPool::get_connection_number() const
{
    return (uint16_t)_connection_queue->size();
}

//////////////////////////////////////////////////////////////////////////
// CMySQLConnectionFactory

sys::IDBConnection* CMySQLConnectionFactory::create_connection(const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password)
{
    CMySQLGeneralConnection* db_connection = new CMySQLGeneralConnection;
    try
    {
        db_connection->open(db_ip, db_port, db_name, db_user, db_password);
        return db_connection;
    }
    catch (sys::CDBException& ex)
    {
        delete db_connection;
        throw;
    }    
}

sys::IDBConnectionPool* CMySQLConnectionFactory::create_connection_pool()
{
    return new CMySQLConnectionPool();
}

void CMySQLConnectionFactory::destroy_connection_pool(sys::IDBConnectionPool*& db_connection_pool)
{
    delete (CMySQLConnectionPool*)db_connection_pool;
    db_connection_pool = NULL;
}

//////////////////////////////////////////////////////////////////////////
// 出口函数

static sys::CLock g_mysql_factory_lock;
static CMySQLConnectionFactory* g_mysql_connection_factory = NULL;
sys::IDBConnectionFactory* get_mysql_connection_factory()
{
    if (NULL == g_mysql_connection_factory)
    {
        sys::LockHelper<sys::CLock> lock(g_mysql_factory_lock);
        if (NULL == g_mysql_connection_factory)
            g_mysql_connection_factory = new CMySQLConnectionFactory;
    }

    return g_mysql_connection_factory;
}

LIBPLUGIN_NAMESPACE_END
