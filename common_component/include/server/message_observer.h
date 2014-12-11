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
#ifndef MOOON_SERVER_MESSAGE_OBSERVER_H
#define MOOON_SERVER_MESSAGE_OBSERVER_H
#include <server/packet_handler.h>
#include <net/inttypes.h>
SERVER_NAMESPACE_BEGIN

/***
  * 消息观察者
  * 收到一个完整的消息时调用
  * 如果你的消息头和net::TCommonMessageHeader一致，
  * 则建议使用IMessageObserver，而不是IPacketHandler,
  * IMessageObserver相对于IPacketHandler是更高级别的接口
  */
class CALLBACK_INTERFACE IMessageObserver
{
public:
    virtual ~IMessageObserver() {}

    /***
      * 收到一个完整消息时被回调
      * @request_header 输入参数，收到的消息头
      * @request_body 输入参数，收到的消息体
      *  这里需要注意，框架不会释放request_body的内存，需要使用者去释放
      *  释放方法为：delete []request_body;，否则将有内存泄漏
      * @response_buffer 输出参数，发送给对端的响应，默认值为NULL
      *  请注意*response_buffer必须是new char[]出来的，
      *  并且将由框架delete []它
      * @response_size 输出参数，需要发送给对端的响应数据字节数，默认值为0
      * @return 处理成功返回true，否则返回false
      */
    virtual bool on_message(const net::TCommonMessageHeader& request_header
                          , const char* request_body
                          , char** response_buffer
                          , size_t* response_sizer) = 0;

    /***
      * 连接被关闭
      */
    virtual void on_connection_closed()
    {
    }

    /***
      * 连接超时
      * @return 如果返回true，确认是连接超时，连接将被关闭
      *        ；否则表示并未超时，连接会继续使用，同时时间戳会被更新
      */
    virtual bool on_connection_timeout()
    {
        return true;
    }

    /***
     * 包发送完后被回调
     * @return util::handle_continue 表示不关闭连接继续使用，
     *         返回其它值则会关闭连接
     */
    virtual util::handle_result_t on_response_completed()
    {
        //return util::handle_close; // 短连接时
        return util::handle_continue; // 长连接时
    }
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_MESSAGE_OBSERVER_H
