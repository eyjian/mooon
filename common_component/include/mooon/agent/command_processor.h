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
#ifndef MOOON_AGENT_COMMAND_PROCESSOR_H
#define MOOON_AGENT_COMMAND_PROCESSOR_H
#include <mooon/agent/message.h>
AGENT_NAMESPACE_BEGIN

/***
  * 消息上下文结构
  * 由于是异步接收消息的，所以需要一个上下文结构来保存最新状态
  */
typedef struct TMessageContext
{
    size_t total_size;   /** 消息体的字节数 */
    size_t finished_size; /** 已经收到的消息体字节数 */
    
    TMessageContext(size_t total_size_, size_t finished_size_)
     :total_size(total_size_)
     ,finished_size(finished_size_)
    {
    }
}message_context_t;

class ICommandProcessor
{ 
public:
    virtual ~ICommandProcessor() {}

    /***
      * 返回该CommandProcessor处理的命令字
      */
    virtual uint32_t get_command() const = 0;

    /***
      * 解析出一个完整消息头时回调用
      */
    virtual bool on_header(const net::TCommonMessageHeader& header)
    {
        return true;
    }

    /***
	  * 有消息需要处理时的回调函数
	  * 请注意消息的接收是异步的，每收到一点消息数据，都会回调on_message
	  * 整个消息包接收完成的条件是msg_ctx.total_size和msg_ctx.finished_size+buffer_size两者相等
	  * @buffer 当前收到的消息体数据
	  * @buffer_size 当前收到的消息体数据字节数
	  * @return 如果消息处理成功，则返回true，否则返回false，当返回false时，会导致连接被断开进行重连接
	  */
    virtual bool on_message(const TMessageContext& msg_ctx, const char* buffer, size_t buffer_size) = 0;
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_COMMAND_PROCESSOR_H

