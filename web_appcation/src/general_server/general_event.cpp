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
#include <sys/log.h>
#include "general_event.h"
MOOON_NAMESPACE_BEGIN

/** 复位操作 */
void CGeneralEvent::reset()
{    
}

/** 已经解析到包头尾 */
bool CGeneralEvent::on_head_end()
{
    MYLOG_DEBUG("Http header end.\n");
    return true;
}

/***
  * 解析出错
  * @errmsg: 错误信息
  */
void CGeneralEvent::on_error(const char* errmsg)    
{
    MYLOG_DEBUG("Parse http error: %s.\n", errmsg);
}

/***
  * 已经解析出的HTTP方法
  * @begin: 方法名开始位置
  * @end: 方法名结束位置
  * @return: 如果方法正确返回true，否则返回false
  */
bool CGeneralEvent::on_method(const char* begin, const char* end)
{
    MYLOG_DEBUG("Method is %.*s.\n", (int)(end-begin), begin);
    return true;
}

/***
  * 已经解析出的URL
  * @begin: URL开始位置
  * @end: URL结束位置
  * @return: 如果URL正确返回true，否则返回false
  */
bool CGeneralEvent::on_url(const char* begin, const char* end)
{
    MYLOG_DEBUG("Url is %.*s.\n", (int)(end-begin), begin);
    return true;
}

/***
  * 已经解析出的版本号，如HTTP/1.1
  * @begin: 版本号开始位置
  * @end: 版本号结束位置
  * @return: 如果版本号正确返回true，否则返回false
  */
bool CGeneralEvent::on_version(const char* begin, const char* end)
{
    MYLOG_DEBUG("Version is %.*s.\n", (int)(end-begin), begin);
    return true;
}

/***
  * 已经解析出的响应代码
  * @begin: 响应代码开始位置
  * @end: 响应代码结束位置
  * @return: 如果响应代码正确返回true，否则返回false
  */
bool CGeneralEvent::on_code(const char* begin, const char* end)
{
    return true;
}

/***
  * 已经解析出的响应代码描述，如OK
  * @begin: 响应代码描述开始位置
  * @end: 响应代码描述结束位置
  * @return: 如果响应代码描述正确返回true，否则返回false
  */
bool CGeneralEvent::on_describe(const char* begin, const char* end)
{    
    return true;
}

/***
  * 已经解析出的名值对，如：host: www.hadoopor.com
  * @name_begin: 名字的开始位置
  * @name_end: 名字的结束位置
  * @value_begin: 值的开始位置
  * @value_end: 值的结束位置
  * @return: 如果名值对正确返回true，否则返回false
  */
bool CGeneralEvent::on_name_value_pair(const char* name_begin, const char* name_end
                               ,const char* value_begin, const char* value_end)
{
    MYLOG_DEBUG("%.*s ==> %.*s.\n", (int)(name_end-name_begin), name_begin, (int)(value_end-value_begin), value_begin);
    return true;
}

MOOON_NAMESPACE_END
