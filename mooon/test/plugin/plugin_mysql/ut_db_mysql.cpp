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
 */
#include "mooon/sys/db.h"
#include "mooon/sys/ref_countable.h"
#include "mooon/plugin/plugin_mysql/plugin_mysql.h"

using namespace mooon;

// 演示连接池IDBPoolConnection和一般连接IDBConnection的使用

// 往标准输出按行输出表中的所有记录
template <class DBConnectionClass>
void print_table(DBConnectionClass* db_connection, const char* sql)
{
    size_t row = 0; // 当前行数

    // 执行一条查询语句
    sys::IRecordset* recordset = db_connection->query("%s", sql);
    uint16_t field_number = recordset->get_field_number();

    // 自动释放
    sys::RecordsetHelper<DBConnectionClass> recordset_helper(db_connection, recordset);
    
    for (;;)
    {
        // 取下一行记录
        sys::IRecordrow* recordrow = recordset->get_next_recordrow();
        if (NULL == recordrow) break;

        // 自动释放
        sys::RecordrowHelper recordrow_helper(recordset, recordrow);

        // 循环打印出所有字段值
        fprintf(stdout, "ROW[%04d] ==>\t", row++);
        for (uint16_t col=0; col<field_number; ++col)
        {
            const char* field_value = recordrow->get_field_value(col);
            fprintf(stdout, "%s\t", field_value);
        }
        fprintf(stdout, "\n");
    }
}

int main()
{
    std::string sql = "SELECT * FROM test"; // 需要查询的SQL语句
    std::string db_ip = "127.0.0.1";
    std::string db_name = "test";
    std::string db_user = "root";
    std::string db_password = "";
    
    // 得到连接工厂
    sys::IDBConnectionFactory* db_connection_factory = libplugin::get_mysql_connection_factory();

    try
    {
        // 创建一个一般连接
        sys::IDBConnection* general_connection = db_connection_factory->create_connection
                                (db_ip.c_str(), 3306, db_name.c_str(), db_user.c_str(), db_password.c_str());

        // 使用引用计数帮助类，自动进行引用计数增一和减一
        sys::CRefCountHelper<sys::IDBConnection> rch(general_connection);
        
        // 创建一个连接池
        sys::IDBConnectionPool* db_connection_pool = db_connection_factory->create_connection_pool();
        sys::DBConnectionPoolHelper dph(db_connection_factory, db_connection_pool); // 用于自动销毁数据库连接池
            
        // 创建数据库连接池
        db_connection_pool->create(10, db_ip.c_str(), 3306, db_name.c_str(), db_user.c_str(), db_password.c_str());
    
        do // 这个循环无实际意义，仅为简化代码结构
        {
            // 从数据库连接池中取一个连接
            sys::IDBPoolConnection* pool_connection = db_connection_pool->get_connection();
            if (NULL == pool_connection)
            {
                fprintf(stderr, "Database pool is empty.\n");
                break;
            }

            // 自动释放
            sys::DBPoolConnectionHelper db_connection_helper(db_connection_pool, pool_connection);
        
            printf("The following are from General Connection:\n");
            print_table<sys::IDBConnection>(general_connection, sql.c_str());

            printf("\n\nThe following are from Pool Connection:\n");
            print_table<sys::IDBPoolConnection>(pool_connection, sql.c_str());
        } while(false);
    }
    catch (sys::CDBException& ex)
    {
        fprintf(stderr, "Create database connection pool error: %s.\n", ex.get_error_message());
    }    

    return 0;
}
