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
#include "agent_connector.h"
#include <net/util.h>
#include "agent_log.h"
#include "agent_thread.h"
AGENT_NAMESPACE_BEGIN

CAgentConnector::CAgentConnector(CAgentThread* thread)
 :_thread(thread)
 ,_send_machine(this)
 ,_recv_machine(thread->get_processor_manager())
{
}

void CAgentConnector::before_close()
{
    _recv_machine.reset();
    _send_machine.reset(true);
}

net::epoll_event_t CAgentConnector::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{
    net::epoll_event_t handle_result = net::epoll_close;
    
    try
    {
        if (events & EPOLLIN)
        {
            handle_result = handle_input(input_ptr, ouput_ptr);
        }
        else if (events & EPOLLOUT)
        {
            handle_result = handle_output(input_ptr, ouput_ptr);
        }
        else
        {
            handle_result = handle_error(input_ptr, ouput_ptr);
        }
    }
    catch (sys::CSyscallException& ex)
    {
        AGENT_LOG_ERROR("%s exception(%u): %s.\n", to_string().c_str(), events, ex.to_string().c_str());
    }
    
    return handle_result;
}

net::epoll_event_t CAgentConnector::handle_error(void* input_ptr, void* ouput_ptr)
{
    return net::epoll_close;
}

net::epoll_event_t CAgentConnector::handle_input(void* input_ptr, void* ouput_ptr)
{
    char recv_buffer[1024];
    
    ssize_t bytes_recved = receive(recv_buffer, sizeof(recv_buffer));
    if (0 == bytes_recved)
    {
    	AGENT_LOG_DEBUG("%s closed.\n", to_string().c_str());
    	return net::epoll_close;
    }
    if (-1 == bytes_recved)
    {
        // Would block
        AGENT_LOG_DEBUG("%s receive would block.\n", to_string().c_str());
        return net::epoll_none;
    }
    
    return util::handle_error == _recv_machine.work(recv_buffer, bytes_recved)
         ? net::epoll_close
         : net::epoll_none;
}

net::epoll_event_t CAgentConnector::handle_output(void* input_ptr, void* ouput_ptr) 
{
    util::handle_result_t hr = util::handle_finish;
    
    // 如果上次有未发送完的，则先保证原有的发送完
    if (!_send_machine.is_finish())
    {
        hr = _send_machine.continue_send();
    }
    
    // 发送新的消息
    if (util::handle_finish == hr)
    {
        _send_machine.reset(true);

        while (true)
        {
            const net::TCommonMessageHeader* agent_message = _thread->get_message();
            if (NULL == agent_message)
            {
                // 需要将CReportQueue再次放入Epoller中监控
                AGENT_LOG_DEBUG("No message to send.\n");
                _thread->enable_queue_read();
                hr = util::handle_finish;
                break;
            }
            
            size_t bytes_sent = sizeof(net::TCommonMessageHeader) + agent_message->size;
            AGENT_LOG_DEBUG("Will send %zu bytes\n", bytes_sent);
            hr = _send_machine.send(reinterpret_cast<const char*>(agent_message), bytes_sent);
            if (util::handle_finish == hr)
            {
                _send_machine.reset(true);
            }
            else
            {
                break;
            }
        }
    }
    
    // 转换返回值
    if (util::handle_error == hr)
    {
        return net::epoll_close;
    }
    else
    {
        return util::handle_continue == hr
             ? net::epoll_read_write
             : net::epoll_read;
    }
}

AGENT_NAMESPACE_END
