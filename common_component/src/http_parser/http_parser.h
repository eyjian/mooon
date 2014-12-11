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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef MOOON_HTTP_PARSER_H
#define MOOON_HTTP_PARSER_H
#include <util/config.h>

/***
  * http parser模块的名字空间名称定义
  */
#define HTTP_PARSER_NAMESPACE_BEGIN namespace mooon { namespace http_parser {
#define HTTP_PARSER_NAMESPACE_END                   }}
#define HTTP_PARSER_NAMESPACE_USE using mooon::http_parser;

//////////////////////////////////////////////////////////////////////////
HTTP_PARSER_NAMESPACE_BEGIN

/***
  * Http事件回调接口
  */
class CALLBACK_INTERFACE IHttpEvent
{
public:    
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IHttpEvent() {}

    /** 复位操作 */
    virtual void reset() {}

    /** 已经解析到包头尾 */
    virtual bool on_head_end() { return true; }
    
    /***
      * 解析出错
      * @errmsg: 错误信息
      */
    virtual void on_error(const char* errmsg) {}   

    /***
      * 已经解析出的HTTP方法
      * @begin: 方法名开始位置
      * @end: 方法名结束位置
      * @return: 如果方法正确返回true，否则返回false
      */
    virtual bool on_method(const char* begin, const char* end) { return true; }

    /***
      * 已经解析出的URL
      * @begin: URL开始位置
      * @end: URL结束位置
      * @return: 如果URL正确返回true，否则返回false
      */
    virtual bool on_url(const char* begin, const char* end) { return true; }

    /***
      * 已经解析出的版本号，如HTTP/1.1
      * @begin: 版本号开始位置
      * @end: 版本号结束位置
      * @return: 如果版本号正确返回true，否则返回false
      */
    virtual bool on_version(const char* begin, const char* end) { return true; }

    /***
      * 已经解析出的响应代码
      * @begin: 响应代码开始位置
      * @end: 响应代码结束位置
      * @return: 如果响应代码正确返回true，否则返回false
      */
    virtual bool on_code(const char* begin, const char* end) { return true; }

    /***
      * 已经解析出的响应代码描述，如OK
      * @begin: 响应代码描述开始位置
      * @end: 响应代码描述结束位置
      * @return: 如果响应代码描述正确返回true，否则返回false
      */
    virtual bool on_describe(const char* begin, const char* end) { return true; }

    /***
      * 已经解析出的名值对，如：host: www.hadoopor.com
      * @name_begin: 名字的开始位置
      * @name_end: 名字的结束位置
      * @value_begin: 值的开始位置
      * @value_end: 值的结束位置
      * @return: 如果名值对正确返回true，否则返回false
      */
    virtual bool on_name_value_pair(const char* name_begin, const char* name_end
                                  , const char* value_begin, const char* value_end)
    {
        return true;
    }
};

/***
  * HTTP协议解析器接口
  * 采用流式递增解析方法，因为解析总是向前，不会回溯，
  * 因此支持接收一部分(最小为一个字节)数据，就进行这部分的解析
  */
class IHttpParser
{
public:    
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IHttpParser() {}

    /***
      * 复位解析状态
      * 请注意当reset后，head_finished将返回false，get_head_length将返回0，
      * 直到再次调用parse
      */
	virtual void reset() = 0;

    /** 包头已经解析完成 */
    virtual bool head_finished() const = 0;

    /** 得到包头的字节数 */
    virtual int get_head_length() const = 0;

    /** 得到HTTP事件 */
    virtual IHttpEvent* get_http_event() const = 0;

    /** 设置HTTP事件 */
    virtual void set_http_event(IHttpEvent* event) = 0;

    /***
      * 执行解析
      * @buffer: 需要解析的Buffer
      * @return: 请参考TReturnResult的说明
      */
    virtual util::handle_result_t parse(const char* buffer) = 0;
};

//////////////////////////////////////////////////////////////////////////

/** 销毁HTTP协议解析器 */
extern IHttpParser* create(bool is_request);

/** 创建HTTP协议解析器 */
extern void destroy(IHttpParser* parser);

HTTP_PARSER_NAMESPACE_END
#endif // MOOON_HTTP_PARSER_H
