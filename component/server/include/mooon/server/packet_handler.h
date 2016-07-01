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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_SERVER_PACKET_HANDLER_H
#define MOOON_SERVER_PACKET_HANDLER_H
#include <mooon/server/config.h>
#include <sys/epoll.h>
#include <sstream>
SERVER_NAMESPACE_BEGIN

/***
  * 下一步动作指标器
  */
struct Indicator
{
    bool reset;             /** 是否复位状态 */
    uint16_t thread_index;  /** 下一步跳到的线程顺序号 */
    uint32_t epoll_events;  /** 下一步注册的epoll事件，可取值EPOLLIN或EPOLLOUT，或EPOLLIN|EPOLLOUT */
};

/***
  * 请求上下文
  */
struct RequestContext
{
    size_t request_size;   /** request_buffer的大小，request_size-request_offset就是本次最大接收的字节数 */
    size_t request_offset; /** 接收到数据时，存入request_buffer的偏移位置 */
    char* request_buffer;  /** 用来接收请求数据的Buffer */

    RequestContext()
    {
        reset();
    }

    void reset()
    {
        request_size   = 0;
        request_offset = 0;
        request_buffer = NULL;
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "request_context://"
           << request_size << "|"
           << request_offset << "|"
           << &request_buffer;

        return ss.str();
    }
};

/***
  * 响应上下文
  */
struct ResponseContext
{
    bool is_response_fd;       /** 是响应一个文件句柄，还是一个Buffer */

    size_t response_size;      /** 本次需要响应的总字节数 */
    size_t response_offset;    /** 从哪个偏移位置开始发送 */

    union
    {
        int response_fd;       /** 文件句柄 */
        char* response_buffer; /** 需要发送的数据 */
    };

    ResponseContext()
    {
        reset();
    }

    void reset()
    {
        is_response_fd  = false;
        response_size   = 0;
        response_offset = 0;
        response_buffer = NULL;
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "response_context://"
           << std::boolalpha << is_response_fd << "|"
           << response_size << "|"
           << response_offset << "|"
           << response_fd << "|"
           << &response_buffer;

        return ss.str();
    }
};

/***
  * 包处理器，包括对请求和响应的处理
  * 如果你的消息头和net::TCommonMessageHeader一致，
  * 则建议使用IMessageObserver，而不是IPacketHandler,
  * IMessageObserver相对于IPacketHandler是更高级别的接口
  */
class CALLBACK_INTERFACE IPacketHandler
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IPacketHandler()
    {
    }

    /***
      * 对收到的数据进行解析
      * @param indicator.reset 默认值为false
      *        indicator.thread_index 默认值为当前线程顺序号
      *        indicator.epoll_events 默认值为EPOLLOUT
      * @data_size: 新收到的数据大小
      * @return util::handle_continue 表示请求未接收完整，需要继续接收
      *         util::handle_finish 表示请求已经接收完整，可进入响应过程了
      *         util::handle_release表示需要对连接进行线程切换
      *         其它值表示连接出错，需要关闭连接
      */
    virtual utils::handle_result_t on_handle_request(size_t data_size, Indicator& indicator) = 0;

    /***
      * 复位解析状态
      */
    virtual void reset()
    {
        //_request_context.reset();
        //_response_context.reset();
    }
    
    /***
      * IO错误发生时被回调
      */
    virtual void on_io_error()
    {
    }

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
      * 进行线程切换失败，连接在调用后将被关闭
      * @overflow 是否因为队列满导致的切换失败，否则是因为目标线程不存在
      */
    virtual void on_switch_failure(bool overflow)
    {
    }

    /***
      * 移动偏移
      * @offset: 本次发送的字节数
      */
    virtual void move_response_offset(size_t offset)
    {
        _response_context.response_offset += offset;
    }

    /***
      * 开始响应前的事件
      */
    virtual void before_response()
    {
    }

    /***
     * 包发送完后被回调
     * @param indicator.reset 默认值为true
     *        indicator.thread_index 默认值为当前线程顺序号
     *        indicator.epoll_events 默认值为EPOLLIN
     * @return util::handle_continue 表示不关闭连接继续使用；
     *         util::handle_release 表示进行线程切换，
     *         IPacketHandler将交给indicator.thread_index指定的线程调度；
     *         返回其它值则关闭连接
     */
    virtual utils::handle_result_t on_response_completed(Indicator& indicator)
    {
        return utils::handle_continue;
    }

public:
    /***
      * 返回指向请求的上下文指针
      */
    RequestContext* get_request_context()
    {
        return &_request_context;
    }

    /***
      * 返回指向响应的上下文指针
      */
    const ResponseContext* get_response_context() const
    {
        return &_response_context;
    }

protected:
    RequestContext _request_context;   /** 用来接收请求的上下文，子类应当修改它 */
    ResponseContext _response_context; /** 用来发送响应的上下文，子类应当修改它 */
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_PACKET_HANDLER_H
