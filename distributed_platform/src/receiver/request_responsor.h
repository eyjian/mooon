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
#ifndef MOOON_RECEIVER_REQUEST_RESPONSOR_H
#define MOOON_RECEIVER_REQUEST_RESPONSOR_H
#include <server/request_responsor.h>
MOOON_NAMESPACE_BEGIN

class CRequestResponsor: public IRequestResponsor
{
public:
    CRequestResponsor(CProtocolParser* parser);

private:
    /** 复位状态 */
    virtual void reset();
    
    /** 是否保持连接不断开，继续下一个请求 */
    virtual bool keep_alive() const;

    /** 是否发送一个文件 */
    virtual bool is_send_file() const;
    
    /** 得到需要发送的大小 */
    virtual uint32_t get_size() const;

    /** 得到从哪偏移开始发送 */
    virtual uint32_t get_offset() const; 

    /** 得到文件句柄 */
    virtual int get_fd() const;              

    /** 得到需要发送的数据 */
    virtual const char* get_buffer() const;

    /***
      * 移动偏移
      * @offset: 本次发送的字节数
      */
    virtual void move_offset(uint32_t offset);

private:
    CProtocolParser* _parser;
};

MOOON_NAMESPACE_END
#endif // MOOON_RECEIVER_REQUEST_RESPONSOR_H
