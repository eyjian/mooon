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
#include <mooon/agent/message.h>
#include <mooon/server/server.h>
#include <mooon/net/recv_machine.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>

// 提供两种实现，一种基于server::IMessageObserver，
// 另一种基于server::IPacketHandler，前者简单，后者复杂
//#define USE_PACKET_HANDLER

STRING_ARG_DEFINE(ip, "127.0.0.1", "listen IP");
INTEGER_ARG_DEFINE(uint16_t, port, 10000, 2048, 65535, "listen port");

AGENT_NAMESPACE_BEGIN

#pragma pack(4)
typedef struct TResponseMessage
{
    net::TCommonMessageHeader header;
    char data[1024];
}response_message_t;
#pragma pack()

class CConfig: public server::IConfig
{
public:
    void add_ipnode(const std::string& ip_address, uint16_t port)
    {
        net::ip_port_pair_t ip_port_pair;
        ip_port_pair.first = ip_address.c_str();
        ip_port_pair.second = port;
        
        _ip_port_pair_array.push_back(ip_port_pair);
    }
    
private:
    virtual uint32_t get_connection_timeout_seconds() const 
    { 
        return 20; 
    }
    
    virtual uint32_t get_connection_pool_size() const
    {
        return 1;
    }
    
    virtual const net::ip_port_pair_array_t& get_listen_parameter() const
    {
        return _ip_port_pair_array;
    }
    
private:
    net::ip_port_pair_array_t _ip_port_pair_array;
};

#if !defined(USE_PACKET_HANDLER)
class CMessageObserver: public server::IMessageObserver
{
public:
    CMessageObserver(server::IConnection* connection)
     :_connection(connection)
    {
    }

private:
    virtual bool on_message(const net::TCommonMessageHeader& request_header
                          , const char* request_body
                          , char** response_buffer
                          , size_t* response_size)
    {
        fprintf(stdout, "command=%u, total size=%u: %s\n"
              , request_header.command.to_int(), request_header.size.to_int()
              , request_body);
        delete []request_body; // 别忘记了

        *response_size = sizeof("mooon") + sizeof(net::TCommonMessageHeader);
        *response_buffer = new char[*response_size];
        response_message_t* response_message = reinterpret_cast<response_message_t*>(*response_buffer);

        response_message->header.size = *response_size - sizeof(net::TCommonMessageHeader);
        response_message->header.command = request_header.command.to_int();
        strcpy(response_message->data, "mooon");

        return true;
    }

private:
    server::IConnection* _connection;
};

#else

class CMessageHandler
{
public:
	CMessageHandler()
	 :_command(0)
	{
	}

	uint32_t get_command() const
	{
		return _command;
	}

	bool on_header(const net::TCommonMessageHeader& header)
	{
	    return true;
	}

    bool on_message(const net::TCommonMessageHeader& header
                  , size_t finished_size
                  , const char* buffer, size_t buffer_size)
    {
    	_command = header.command + 1;
        fprintf(stdout, "command=%u, total size=%u, finished size=%zu: %s\n"
              , header.command.to_int(), header.size.to_int(), finished_size
              , buffer);
        return true;
    }

private:
    uint32_t _command;
};

class CPacketHandler: public server::IPacketHandler
{
public:
    CPacketHandler(server::IConnection* connection)
     :_connection(connection)
     ,_recv_machine(&_message_handler)
    {
        _request_context.request_buffer = new char[sys::CUtil::get_page_size()];
        _request_context.request_size = sys::CUtil::get_page_size() - 1;
        _request_context.request_offset = 0;
        
        _response_context.is_response_fd = false;
        _response_context.response_buffer = reinterpret_cast<char*>(&_response_message);
        _response_context.response_size = sizeof(net::TCommonMessageHeader) + _response_message.header.size.to_int();
        _response_context.response_offset = 0;

        _response_message.header.command = 1;
        _response_message.header.size = sizeof("success");
        strcpy(_response_message.data, "success");
    }
    
    ~CPacketHandler()
    {
        delete []_request_context.request_buffer;
    }
    
private:
    virtual void before_response()
    {
        _response_context.response_offset = 0;
        _response_message.header.command = _message_handler.get_command();
    }
    
    virtual util::handle_result_t on_handle_request(size_t data_size, server::Indicator& indicator)
    {
        return _recv_machine.work(_request_context.request_buffer, data_size);
    }
    
private:
    response_message_t _response_message;
    server::IConnection* _connection;
    CMessageHandler _message_handler;
    net::CRecvMachine<net::TCommonMessageHeader, CMessageHandler> _recv_machine;
};
#endif // USE_PACKET_HANDLER

class CFactory: public server::IFactory
{
private:
    virtual server::IPacketHandler* create_packet_handler(server::IConnection* connection)
    {
#if !defined(USE_PACKET_HANDLER)
        return NULL;
#else
        return new CPacketHandler(connection);
#endif // USE_PACKET_HANDLER
    }

    virtual server::IMessageObserver* create_message_observer(server::IConnection* connection)
    {
#if !defined(USE_PACKET_HANDLER)
        return new CMessageObserver(connection);
#else
        return NULL;
#endif // USE_PACKET_HANDLER
    }
};

class CMainHelper: public sys::IMainHelper
{   
public:
    CMainHelper()
     :_server(NULL)
    {
    }
    
private:
    virtual bool init(int argc, char* argv[])
    {
        _config.add_ipnode(mooon::argument::ip->value()
                          ,mooon::argument::port->value());
                
        return true;
    }

    virtual bool run()
    {
        _server = server::create(&_config, &_factory);
        return _server != NULL;
    }
    
    virtual void fini()
    {
        server::destroy(_server);
    }
    
    virtual int get_exit_signal() const
    {
    	return SIGTERM;
    }

private:
    CConfig _config;
    CFactory _factory;
    server::server_t _server;
};

extern "C" int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }
    
    CMainHelper main_helper;
    return sys::main_template(&main_helper, argc, argv);
}

AGENT_NAMESPACE_END
