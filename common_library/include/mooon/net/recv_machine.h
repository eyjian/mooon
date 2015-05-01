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
#ifndef MOOON_NET_RECV_MACHINE_H
#define MOOON_NET_RECV_MACHINE_H
#include <mooon/net/config.h>
NET_NAMESPACE_BEGIN

/***
  * @MessageHeaderType 消息头类型，要求是固定大小的，必须包含名为size的成员
  * @ProcessorManager 能够针对指定消息进行处理的类，为什么取名为Manager，
  *                   因为通常不同的消息由不同的Processor处理
  *  ProcessorManager必须包含如下方法，当解析出一个完整的包后，会调用它：
  *  bool on_header(const MessageHeaderType& header); // 解析出一个包头后被调用
  *  bool on_message(                                 // 每收到一点消息体时，都会被调用
  *          const MessageHeaderType& header // 包头，包头的size为包体大小，不包含header本身
           , size_t finished_size            // 已经接收到的大小
           , const char* buffer              // 当前收到的数据
           , size_t buffer_size);            // 当前收到的字节数
  */
template <typename MessageHeaderType, class ProcessorManager>
class CRecvMachine
{
private:
    /***
      * 接收状态值
      */
    typedef enum TRecvState
    {
        rs_header = 0, /** 接收消息头状态 */
        rs_body   = 1 /** 接收消息体状态 */
    }recv_state_t;

    /***
      * 接收状态上下文
      */
    struct RecvStateContext
    {
        const char* buffer; /** 当前的数据buffer */
        size_t buffer_size; /** 当前的数据字节数 */

        RecvStateContext(const char* buf=NULL, size_t buf_size=0)
         :buffer(buf)
         ,buffer_size(buf_size)
        {
        }

        RecvStateContext(const RecvStateContext& other)
         :buffer(other.buffer)
         ,buffer_size(other.buffer_size)
        {
        }

        RecvStateContext& operator =(const RecvStateContext& other)
        {
            buffer = other.buffer;
            buffer_size = other.buffer_size;
            return *this;
        }
    };

public:
    CRecvMachine(ProcessorManager* processor_manager);

    // 状态机入口函数
    // 状态机工作原理：-> rs_header -> rs_body -> rs_header
    //                 -> rs_header -> rs_error -> rs_header
    //                 -> rs_header -> rs_body -> rs_error -> rs_header
    // 参数说明：
    // buffer - 本次收到的数据，注意不是总的
    // buffer_size - 本次收到的数据字节数
    // 返回值：
    // 1) 如果出错，则返回utils::handle_error
    // 2) 如果未到包的边界，则返回utils::handle_continue
    // 3) 如果刚好到包的边界，则返回utils::handle_finish
    // 注意：未到包的边界，并不表示还没有接收到一个完整的包，
    // 因为可能是两个包连着的，第一个包在work过程中已经收完，
    // buffer还包含第二个包的部分时，也是返回utils::handle_continue
    utils::handle_result_t work(const char* buffer, size_t buffer_size);

    // 复位状态，再次以包头开始
    void reset();

private:
    void set_next_state(recv_state_t next_state);
    utils::handle_result_t handle_header(const RecvStateContext& cur_ctx, RecvStateContext* next_ctx);
    utils::handle_result_t handle_body(const RecvStateContext& cur_ctx, RecvStateContext* next_ctx);
    utils::handle_result_t handle_error(const RecvStateContext& cur_ctx, RecvStateContext* next_ctx);

private:
    MessageHeaderType _header; /** 消息头，这个大小是固定的 */
    ProcessorManager* _processor_manager;
    recv_state_t _current_recv_state; /** 当前的接收状态 */
    size_t _finished_size; /** 当前状态已经接收到的字节数，注意不是总的已经接收到的字节数，只针对当前状态 */
};

template <typename MessageHeaderType, class ProcessorManager>
CRecvMachine<MessageHeaderType, ProcessorManager>::CRecvMachine(ProcessorManager* processor_manager)
 :_processor_manager(processor_manager)
{
    reset();
}

template <typename MessageHeaderType, class ProcessorManager>
utils::handle_result_t CRecvMachine<MessageHeaderType, ProcessorManager>::work(
    const char* buffer, 
    size_t buffer_size)
{
    RecvStateContext next_ctx(buffer, buffer_size);
    utils::handle_result_t hr = utils::handle_continue;

    // 状态机循环条件为：utils::handle_continue == hr
    while (utils::handle_continue == hr)
    {
        RecvStateContext cur_ctx(next_ctx);

        switch (_current_recv_state)
        {
        case rs_header:
            hr = handle_header(cur_ctx, &next_ctx);
            break;
        case rs_body:
            hr = handle_body(cur_ctx, &next_ctx);
            break;
        default:
            hr = handle_error(cur_ctx, &next_ctx);
            break;
        }
    }
    if (utils::handle_error == hr)
    {
        set_next_state(rs_header);
    }

    return hr;
}

template <typename MessageHeaderType, class ProcessorManager>
void CRecvMachine<MessageHeaderType, ProcessorManager>::set_next_state(
    recv_state_t next_state)
{
    _current_recv_state = next_state;
    _finished_size = 0;
}

template <typename MessageHeaderType, class ProcessorManager>
void CRecvMachine<MessageHeaderType, ProcessorManager>::reset()
{
    set_next_state(rs_header);
}

// 处理消息头部
// 参数说明：
// cur_ctx - 当前上下文，
//           cur_ctx.buffer为当前收到的数据buffer，包含了消息头，但也可能包含了消息体。
//           cur_ctx.buffer_size为当前收到字节数
// next_ctx - 下一步上下文，
//           由于cur_ctx.buffer可能包含了消息体，所以在一次接收receive动作后，
//           会涉及到消息头和消息体两个状态，这里的next_ctx实际为下一步handle_body的cur_ctx
template <typename MessageHeaderType, class ProcessorManager>
utils::handle_result_t CRecvMachine<MessageHeaderType, ProcessorManager>::handle_header(
    const RecvStateContext& cur_ctx, 
    RecvStateContext* next_ctx)
{
    if (_finished_size + cur_ctx.buffer_size < sizeof(MessageHeaderType))
    {
        memcpy(reinterpret_cast<char*>(&_header) + _finished_size
              ,cur_ctx.buffer
              ,cur_ctx.buffer_size);

        _finished_size += cur_ctx.buffer_size;
        // 本次数据包已接收完
        return utils::handle_finish;
    }
    else
    {
        size_t need_size = sizeof(MessageHeaderType) - _finished_size;
        memcpy(reinterpret_cast<char*>(&_header) + _finished_size
              ,cur_ctx.buffer
              ,need_size);

        // TODO: Check header here
        if (!_processor_manager->on_header(_header))
        {
            return utils::handle_error;
        }

        size_t remain_size = cur_ctx.buffer_size - need_size;
        if (remain_size > 0)
        {
            next_ctx->buffer = cur_ctx.buffer + need_size;
            next_ctx->buffer_size = cur_ctx.buffer_size - need_size;
        }

        // 只有当包含消息体时，才需要进行状态切换，
        // 否则维持rs_header状态不变
        if (_header.size > 0)
        {
            // 切换状态
            set_next_state(rs_body);
        }
        else
        {            
            if (!_processor_manager->on_message(_header, 0, NULL, 0))
            {
                return utils::handle_error;
            }
        }

        return (remain_size > 0)
              ? utils::handle_continue // 控制work过程是否继续循环
              : utils::handle_finish;
    }
}

// 处理消息体
// 参数说明：
// cur_ctx - 当前上下文，
//           cur_ctx.buffer为当前收到的数据buffer，包含了消息体，但也可能包含了消息头。
//           cur_ctx.buffer_size为当前收到字节数
// next_ctx - 下一步上下文，
//           由于cur_ctx.buffer可能包含了消息头，所以在一次接收receive动作后，
//           会涉及到消息头和消息体两个状态，这里的next_ctx实际为下一步handle_header的cur_ctx
template <typename MessageHeaderType, class ProcessorManager>
utils::handle_result_t CRecvMachine<MessageHeaderType, ProcessorManager>::handle_body(
    const RecvStateContext& cur_ctx, 
    RecvStateContext* next_ctx)
{
    if (_finished_size + cur_ctx.buffer_size < _header.size)
    {
        if (!_processor_manager->on_message(_header, _finished_size, cur_ctx.buffer, cur_ctx.buffer_size))
        {
            return utils::handle_error;
        }

        _finished_size += cur_ctx.buffer_size;
        // 本次数据包已接收完
        return utils::handle_finish;
    }
    else
    {
        size_t need_size = _header.size - _finished_size;
        if (!_processor_manager->on_message(_header, _finished_size, cur_ctx.buffer, need_size))
        {
            return utils::handle_error;
        }

        // 切换状态
        set_next_state(rs_header);

        size_t remain_size = cur_ctx.buffer_size - need_size;
        if (remain_size > 0)
        {
            // 下一个包
            next_ctx->buffer = cur_ctx.buffer + need_size;
            next_ctx->buffer_size = cur_ctx.buffer_size - need_size;
            return utils::handle_continue;
        }

        // 刚好一个完整的包
        return utils::handle_finish;
    }
}

template <typename MessageHeaderType, class ProcessorManager>
utils::handle_result_t CRecvMachine<MessageHeaderType, ProcessorManager>::handle_error(
    const RecvStateContext& cur_ctx, 
    RecvStateContext* next_ctx)
{
    set_next_state(rs_header); // 无条件切换到rs_header，这个时候应当断开连接重连接
    return utils::handle_error;
}

NET_NAMESPACE_END
#endif // MOOON_NET_RECV_MACHINE_H
