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
#ifndef MOOON_DISPATCHER_SENDER_H
#define MOOON_DISPATCHER_SENDER_H
#include <sys/uio.h>
#include <net/tcp_client.h>
#include <util/listable.h>
#include <util/timeoutable.h>
#include "send_queue.h"
DISPATCHER_NAMESPACE_BEGIN

class CSendThread;
class CSenderTable;
class CSender: public ISender, public net::CTcpClient, public util::CTimeoutable, public util::CListable<CSender>
{   
    // reset动作
    typedef enum
    { 
        ra_finish,  // 消息全部发送完毕
        ra_error,   // 消息发送出错
        ra_continue // 消息未发送完毕，需要继续发送
    }reset_action_t;

public:    
    CSender(); // 默认构造函数，不做实际用，仅为满足CListQueue的空闲头结点需求
    virtual ~CSender();                
    CSender(const SenderInfo& sender_info);
    
    virtual bool on_timeout();
    virtual std::string to_string() const;
    virtual const SenderInfo& get_sender_info() const { return _sender_info; }
    virtual void set_reconnect_times(int32_t reconnect_times);

    void shutdown();
    bool to_shutdown() const { return _to_shutdown; }

    bool is_in_table() const { return _in_table; }
    void set_in_table(bool in_table) { _in_table = in_table; }

    CSenderTable* get_sender_table() { return _sender_table; }
    void attach_thread(CSendThread* send_thread);
    void attach_sender_table(CSenderTable* sender_table);
    
private:
    virtual void before_close();
    virtual void after_connect();
    virtual void on_connect_failure();
    
private: // ISender
    virtual std::string str() const { return to_string(); }     
    virtual bool push_message(file_message_t* message, uint32_t milliseconds);
    virtual bool push_message(buffer_message_t* message, uint32_t milliseconds);
    
private:
    void clear_message();    
    void inc_resend_times();    
    bool need_resend() const;    
    void reset_resend_times();
    bool get_current_message();    
    void free_current_message();
    void reset_current_message(bool finish);
    util::handle_result_t do_handle_reply();    
    net::epoll_event_t do_send_message(void* input_ptr, uint32_t events, void* output_ptr);
    template <typename ConcreteMessage>
    bool do_push_message(ConcreteMessage* concrete_message, uint32_t milliseconds);
    
protected:    
    CSendThread* get_send_thread() { return _send_thread; }
    net::epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* output_ptr);
       
private:      
    SenderInfo _sender_info;    
    CSendQueue _send_queue;        
    CSendThread* _send_thread;
    CSenderTable* _sender_table;
    
private:
    volatile bool _in_table; // 是否在SendTable中受控
    volatile bool _to_shutdown;
    volatile int _cur_resend_times;    // 当前已经连续重发的次数
    volatile size_t _current_offset;      // 当前已经发送的字节数
    message_t* _current_message; // 当前正在发送的消息
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_SENDER_H
