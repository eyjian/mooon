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
#ifndef MOOON_SYS_SQLITE3_DB_H
#define MOOON_SYS_SQLITE3_DB_H
#include "mooon/sys/simple_db.h"
#include "mooon/utils/object.h"
#include <stdarg.h>

// SQLite3安装后的include目录结构不标准，需要手动调整成：
// 头文件sqlite3.h和sqlite3ext.h放到目录：<安装目录>/include/sqlite3
// 亦即需要在include目录下新建一个sqlite3子目录，然后将sqlite3.h和sqlite3ext.h两个头文件移至到这个目录

#if MOOON_HAVE_SQLITE3==1
SYS_NAMESPACE_BEGIN

/**
 * SQLite3版本的DB连接
 */
class CSQLite3Connection: public CDBConnectionBase, public utils::CObject
{
public:
    CSQLite3Connection(size_t sql_max=8192);
    ~CSQLite3Connection();

public:
    virtual void open() throw (CDBException);
    virtual void close() throw ();
    virtual void reopen() throw (CDBException);

    virtual uint64_t update(const char* format, ...) throw (CDBException) __attribute__((format(printf, 2, 3)));
    virtual std::string str() throw ();

private:
    virtual void do_query(DBTable& db_table, const char* sql, int sql_length) throw (CDBException);

private:
    void do_open() throw (CDBException);

private:
    void* _sqlite;
};

SYS_NAMESPACE_END
#endif // MOOON_HAVE_SQLITE3
#endif // MOOON_SYS_SQLITE3_DB_H
