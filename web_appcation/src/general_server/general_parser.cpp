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
#include "general_event.h"
#include "general_parser.h"
MOOON_NAMESPACE_BEGIN

CGeneralParser::CGeneralParser()
{
    CGeneralEvent* general_event = new CGeneralEvent;
    _http_parser = create_http_parser(true);
    _http_parser->set_http_event(general_event);    

    reset();
}

CGeneralParser::~CGeneralParser()
{
    destroy_http_parser(_http_parser);
}

/***
  * 复位解析状态
  */
void CGeneralParser::reset()
{
    _offset = 0;
    _http_parser->reset();
}

/***
  * 对收到的数据进行解析
  * @data_size: 新收到的数据大小
  */
util::handle_result_t CGeneralParser::parse(uint32_t data_size)
{
    return _http_parser->parse(_buffer+_offset);
}

/***
  * 更新Buffer偏移
  * @offset: 新接收到的数据大小
  */
void CGeneralParser::move_buffer_offset(uint32_t offset)
{
    _offset += offset;
}

/***
  * 得到从哪个位置开始将接收到的数据存储到Buffer
  */
uint32_t CGeneralParser::get_buffer_offset() const
{
    return _offset;
}

/***
  * 得到用来接收数据的Buffer大小
  */
uint32_t CGeneralParser::get_buffer_size() const  
{
    return sizeof(_buffer)-1;
}

/***
  * 得到用来接收数据的Buffer
  */
char* CGeneralParser::get_buffer()
{
    return _buffer;   
}

MOOON_NAMESPACE_END
