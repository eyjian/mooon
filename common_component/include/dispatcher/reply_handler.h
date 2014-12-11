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
#ifndef MOOON_DISPATCHER_REPLY_HANDLER_H
#define MOOON_DISPATCHER_REPLY_HANDLER_H
#include <net/ip_node.h>
#include <net/ip_address.h>
#include <dispatcher/config.h>
DISPATCHER_NAMESPACE_BEGIN

/***
  * 应答消息处理器，每个Sender都会对应一个应答消息处理器
  */
class ISender;
class CALLBACK_INTERFACE IReplyHandler
{
public:
    // 虚析构用于应付编译器
    virtual ~IReplyHandler() {}    
    
    /** 关联到所服务的Sender */
    virtual void attach(ISender* sender) = 0;

    /** 得到存储应答消息的buffer */
    virtual char* get_buffer() = 0;

    /** 得到存储应答消息的buffer大小 */
    virtual size_t get_buffer_length() const = 0;    

    /** 得到Buffer的偏移 */
    virtual size_t get_buffer_offset() const { return 0; }

    /***
      * 每一个消息被发送前调用
      * @sender: 发送者
      */
    virtual void before_send() {}
    
    /***
      * 当前消息已经成功发送完成
      * @sender: 发送者
      */
    virtual void send_completed() {}

    /***
      * 数据发送进度
      * @total 总的需要发送的字节数
      * @finished 总的已经发送的字节数
      * @current 当次发送出去的字节数
      */
    virtual void send_progress(size_t total, size_t finished, size_t current) {}
        
    /***
      * 和目标的连接断开
      * @sender: 发送者
      */
    virtual void sender_closed() {}

    /***
      * Sender超时
      * @return 如果返回true，则Sender将会被删除，否则不做处理
      */
    virtual bool sender_timeout() { return true; }

    /***
      * 和目标成功建立连接
      * @sender: 发送者
      */
    virtual void sender_connected() {}

    /***
      * 连接到目标失败
      * @sender: 发送者
      */
    virtual void sender_connect_failure() {}

    /***
      * 收到了应答数据，进行应答处理
      * @sender: 发送者
      * @data_size: 本次收到的数据字节数
      */
    virtual util::handle_result_t handle_reply(size_t data_size) { return util::handle_error; }

    /***
      * 得到状态值
      */
    virtual int get_state() const { return 0; }

    /***
      * 设置状态值
      */
    virtual void set_state(int state) {}
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_REPLY_HANDLER_H
