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
#include <sstream>
#include <net/util.h>
#include <util/string_util.h>
#include "getter.h"
#include "reply_handler_impl.h"

SINGLETON_IMPLEMENT(CGetter)

CGetter::CGetter()
    :_dispatcher(NULL)
    ,_port(0)
    ,_number_finished(0)
{
}

void CGetter::set_port(uint16_t port)
{
    _port = port;
}

void CGetter::set_url(const std::string& url)
{
    _url = url;
}

void CGetter::set_domain_name(const std::string& domain_name)
{
    _domain_name = domain_name;
}

void CGetter::set_dispatcher(dispatcher::IDispatcher* dispatcher)
{
    _dispatcher = dispatcher;
}

bool CGetter::start()
{
    do
    {
        if (!get_ip_list())
            break;

        if (!send_http_request())
            break;

        if (!wait_response())
            break;

        return true;
    } while(false);

    return false;
}

void CGetter::connect_over(dispatcher::ISender* sender)
{
    dispatcher::IReplyHandler* reply_handler;
    reply_handler = sender->get_sender_info().reply_handler;

    int state = reply_handler->get_state();
    if (state != GETTER_FINISH)
    {
        sys::LockHelper<sys::CLock> lh(_lock);

        reply_handler->set_state(GETTER_ERROR);
        ++_number_finished;
        _event.signal();
    }
}

void CGetter::request_success(dispatcher::ISender* sender)
{
    dispatcher::IReplyHandler* reply_handler;
    reply_handler = sender->get_sender_info().reply_handler;

    sys::LockHelper<sys::CLock> lh(_lock);

    reply_handler->set_state(GETTER_FINISH);
    ++_number_finished;
    _event.signal();
}

std::string CGetter::get_filename() const
{
    std::string filename = util::CStringUtil::extract_filename(_url);
    if (filename.empty())
        filename = "index.htm";

    return filename;
}

bool CGetter::get_ip_list()
{
    std::string errinfo;
    net::string_ip_array_t string_ip_array;
    if (!net::CUtil::get_ip_address(_domain_name.c_str(), string_ip_array, errinfo))
    {
        fprintf(stderr, "Resolve IP from %s error: %s.\n", _domain_name.c_str(), errinfo.c_str());
        return false;
    }

    // 循环将所有字符串格式的IP转换成整数类型的IP
    // ，这里只考虑IPV4地址，而不考虑IPV6地址
    for (int i=0; i<(int)string_ip_array.size(); ++i)
    {
        uint32_t int_ip;
        if (net::CUtil::string_toipv4(string_ip_array[i].c_str(), int_ip))
        {
            _int_ip_array.push_back(int_ip);
        }
    }
    if (_int_ip_array.empty())
    {
        fprintf(stderr, "Can not get any IP from %s.\n", _domain_name.c_str());
    }

    return !_int_ip_array.empty();
}

bool CGetter::send_http_request()
{
    std::stringstream http_request;
    http_request << "GET "
                 << _url
                 << " HTTP/1.1"
                 << "\r\n"
                 << "Host: "
                 << _domain_name
                 << "\r\n"
                 << "User-Agent: mooon getter"
                 << "\r\n"
                 << "Accept-Encoding: identity"
                 << "\r\n"
                 << "\r\n";

    dispatcher::ISender* sender;
    dispatcher::IManagedSenderTable* sender_table;
    
    // 发送的请求
    fprintf(stdout, "%s", http_request.str().c_str());

    sender_table = _dispatcher->get_managed_sender_table();
    for (int i=0; i<(int)_int_ip_array.size(); ++i)
    {
        dispatcher::SenderInfo sender_info;
        dispatcher::buffer_message_t* buffer_message;

        fill_sender_info(sender_info, i, _int_ip_array[i]);        
        buffer_message = create_request_message(http_request.str());
                
        sender_info.reply_handler->set_state(GETTER_WAITING_RESPONSE);
        sender = sender_table->open_sender(sender_info);

        fprintf(stdout, "Push message to %s.\n", sender->str().c_str());
        sender->push_message(buffer_message);
        sender_table->release_sender(sender);
    }

    return true;
}

bool CGetter::wait_response()
{
    while (_number_finished < (int)_int_ip_array.size())
    {
        _event.wait(_lock);
    }

    return true;
}

dispatcher::buffer_message_t* CGetter::create_request_message(const std::string& request)
{
    dispatcher::buffer_message_t* buffer_message;

    buffer_message = dispatcher::create_buffer_message(request.size()+1);
    strcpy(buffer_message->data, request.c_str());

    return buffer_message;
}

void CGetter::fill_sender_info(dispatcher::SenderInfo& sender_info, uint16_t key, uint32_t ip)
{
    sender_info.key = key;
    sender_info.ip_node.port = _port;
    sender_info.ip_node.ip = ip;        
    sender_info.queue_size = 1;
    sender_info.resend_times = 0;
    sender_info.reconnect_times = 0;
    sender_info.reply_handler = new CReplyHandlerImpl(this);
}
