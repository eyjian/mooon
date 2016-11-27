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
#ifndef MOOON_SYS_DB_EXCEPTION_H
#define MOOON_SYS_DB_EXCEPTION_H
#include "mooon/sys/config.h"
#include "mooon/utils/exception.h"

#define THROW_DB_EXCEPTION(sql, errmsg, errcode) \
    throw ::mooon::sys::CDBException(sql, errmsg, errcode, __FILE__, __LINE__)

SYS_NAMESPACE_BEGIN

/***
 * 错误码定义
 */
enum
{
    DB_NOT_SUPPORTED,       // 不支持的功能
    DB_ERROR_TOO_MANY_COLS, // 查询结果返回超出预期的列数（即返回的字段数过多）
    DB_ERROR_TOO_MANY_ROWS  // 查询结果返回超出预期的行数
};

class CDBException: public utils::CException
{
public:
    /***
      * 构造一个异常对象
      * 请注意不应当显示调用构造函数
      * 对于MySQL返回的出错码1062，表示重复插入
      */
    CDBException(const char* sql, const char* errmsg, int errcode=-1, const char* file=__FILE__, int line=__LINE__)
        : CException(errmsg, errcode, file, line)
    {
        if (sql != NULL)
        {
            _sql = sql;        
        }
    }

    CDBException(const char* sql, const std::string& errmsg, int errcode=-1, const char* file=__FILE__, int line=__LINE__)
        : CException(errmsg, errcode, file, line)
    {
        if (sql != NULL)
        {
            _sql = sql;        
        }
    }
    
    CDBException(const std::string& sql, const std::string& errmsg, int errcode=-1, const char* file=__FILE__, int line=__LINE__)
        : CException(errmsg, errcode, file, line)
    {
        _sql = sql;
    }
    
    virtual ~CDBException() throw ()
    {
    }
    
    virtual std::string prefix() const throw ()
    {
        return "db_exception://";
    }

    /** 返回执行出错的SQL语句，如果不是执行SQL语句，则仅返回一个字符串结尾符 */
    const char* sql() const
    {
        return _sql.c_str();
    }
    
private:   
    std::string _sql;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_DB_EXCEPTION_H
