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
#ifndef MOOON_DISPATCHER_WEB_GETTER_H
#define MOOON_DISPATCHER_WEB_GETTER_H
#include <mooon/sys/event.h>
#include <mooon/dispatcher/dispatcher.h>

class CGetter
{
    SINGLETON_DECLARE(CGetter)

public:
    CGetter();

    void set_port(uint16_t port);
    void set_url(const std::string& url);
    void set_domain_name(const std::string& domain_name);
    void set_dispatcher(mooon::dispatcher::IDispatcher* dispatcher);
    bool start();  
    
    void connect_over(mooon::dispatcher::ISender* sender);
    void request_success(mooon::dispatcher::ISender* sender);

    std::string get_filename() const;
    const std::string& get_domain_name() const { return _domain_name; }

private:
    bool get_ip_list();
    bool send_http_request();
    bool wait_response();
    mooon::dispatcher::buffer_message_t* create_request_message(const std::string& request);
    void fill_sender_info(mooon::dispatcher::SenderInfo& sender_info, uint16_t key, uint32_t ip);

private:
    mooon::dispatcher::IDispatcher* _dispatcher;
    uint16_t _port;
    std::string _url;
    std::string _domain_name; 
    mooon::net::int_ip_array_t _int_ip_array;
    int _number_finished;
    mooon::sys::CEvent _event;
    mooon::sys::CLock _lock;
};

#endif // MOOON_DISPATCHER_WEB_GETTER_H
