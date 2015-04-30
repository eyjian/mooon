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
#ifndef MOOON_PLUGIN_MYSQL_CONNECTION_POOL_H
#define MOOON_PLUGIN_MYSQL_CONNECTION_POOL_H
#include "sys/lock.h"
#include "mysql_connection.h"
#include "util/array_queue.h"
LIBPLUGIN_NAMESPACE_BEGIN

/***
  * 数据库连接池接口
  */
class CMySQLConnectionPool: public sys::IDBConnectionPool
{
public:
    CMySQLConnectionPool();

private:
    /***
      * 得到全小写形式的数据库类型名，如：mysql和postgresql等
      */
    virtual const char* get_type_name() const;
    
    /***
      * 从数据库连接池中获取一个连接
      * @return: 如果当前无可用的连接，则返回NULL，否则返回指向数据库连接的指针
      * @exception: 不会抛出任何异常
      */
    virtual sys::IDBPoolConnection* get_connection();

    /***
      * 将已经获取的数据库连接放回到数据库连接池中
      * @exception: 不会抛出任何异常
      */
    virtual void put_connection(sys::IDBPoolConnection* db_connection);

    /***
      * 创建连接池
      * @pool_size: 数据库连接池中的数据库连接个数
      * @db_ip: 需要连接的数据库IP地址
      * @db_port: 需要连接的数据库服务端口号
      * @db_name: 需要连接的数据库池
      * @db_user: 连接数据库用的用户名
      * @db_password: 连接数据库用的密码
      * @exception: 如出错抛出CDBException异常
      */
    virtual void create(uint16_t pool_size, const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password);

    /***
      * 销毁已经创建的数据库连接池
      */
    virtual void destroy();

    /***
      * 得到连接池中的连接个数
      */
    virtual uint16_t get_connection_number() const;

private: // 连接数据库用
    uint16_t _db_port;        /** 数据库服务端口号 */
    std::string _db_ip;       /** 数据库服务IP地址 */
    std::string _db_name;     /** 数据库名 */
    std::string _db_user;     /** 连接数据库的用户名 */
    std::string _db_password; /** 连接数据库的密码 */

private:
    sys::CLock _lock;
    CMySQLPoolConnection* _connect_array;
    utils::CArrayQueue<CMySQLPoolConnection*>* _connection_queue;    
};

/***
  * 数据库连接工厂，用于创建DBGeneralConnection类型的连接
  */
class CMySQLConnectionFactory: public sys::IDBConnectionFactory
{
private:
    /***
      * 创建DBGeneralConnection类型的连接
      * 线程安全
      * @db_ip: 需要连接的数据库IP地址
      * @db_port: 需要连接的数据库服务端口号
      * @db_name: 需要连接的数据库池
      * @db_user: 连接数据库用的用户名
      * @db_password: 连接数据库用的密码
      * @exception: 如出错抛出CDBException异常
      */
    virtual sys::IDBConnection* create_connection(const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password);

    /***
      * 创建数据库连接池
      * @return: 返回指向数据库连接池的指针
      * @exception: 如出错抛出CDBException异常
      */
    virtual sys::IDBConnectionPool* create_connection_pool();

    /***
      * 销毁数据库连接池
      * @db_connection_pool: 指向需要销毁的数据库连接池的指针
      */
    virtual void destroy_connection_pool(sys::IDBConnectionPool*& db_connection_pool);
};

LIBPLUGIN_NAMESPACE_END
#endif // MOOON_PLUGIN_MYSQL_CONNECTION_POOL_H
