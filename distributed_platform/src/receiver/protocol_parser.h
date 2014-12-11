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
 * Author: eyjian@gmail.com, eyjian@qq.com
 *
 */
#ifndef MOOON_RECEIVER_PROTOCOL_PARSER_H
#define MOOON_RECEIVER_PROTOCOL_PARSER_H
#include "scheduler/message.h"
#include "server/protocol_parser.h"
MOOON_NAMESPACE_BEGIN

/***
  * 协议解析器
  */
class CProtocolParser: public IProtocolParser
{
public:
    CProtocolParser();
    ~CProtocolParser();

private:
    /***
      * 复位解析状态
      */
    virtual void reset();

    /***
      * 对收到的数据进行解析
      * @data_size: 新收到的数据大小
      */
    virtual util::handle_result_t parse(uint32_t data_size);

    /***
      * 更新Buffer偏移
      * @offset: 新接收到的数据大小
      */
    virtual void move_buffer_offset(uint32_t offset);

    /***
      * 得到从哪个位置开始将接收到的数据存储到Buffer
      */
    virtual uint32_t get_buffer_offset() const;

    /***
      * 得到用来接收数据的Buffer大小
      */
    virtual uint32_t get_buffer_size() const;    

    /***
      * 得到用来接收数据的Buffer
      */
    virtual char* get_buffer();

private:
    char* _packet;    
    bool _to_receive_packet; /** 是否去消息包 */
    uint32_t _current_offset;
    first_four_bytes_t _first_four_bytes;
};

MOOON_NAMESPACE_END
#endif // MOOON_RECEIVER_PROTOCOL_PARSER_H
