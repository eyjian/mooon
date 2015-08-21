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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com
 * 用以简化thrift的使用
 * 注意依赖boost：thrift的接口中有使用到boost::shared_ptr
 */
#ifndef MOOON_NET_THRIFT_HELPER_H
#define MOOON_NET_THRIFT_HELPER_H
#include <mooon/net/config.h>
#include <mooon/utils/scoped_ptr.h>
#include <arpa/inet.h>
#include <boost/scoped_ptr.hpp>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TSocketPool.h>
#include <thrift/transport/TTransportException.h>
NET_NAMESPACE_BEGIN

// 用来判断thrift是否已经连接，包括两种情况：
// 1.从未连接过，也就是还未打开过连接
// 2.连接被对端关闭了
inline bool thrift_not_connected(
        apache::thrift::transport::TTransportException::TTransportExceptionType type)
{
    return (apache::thrift::transport::TTransportException::NOT_OPEN == type)
        || (apache::thrift::transport::TTransportException::END_OF_FILE == type);
}

inline bool thrift_not_connected(
        apache::thrift::transport::TTransportException& ex)
{
    apache::thrift::transport::TTransportException::TTransportExceptionType type = ex.getType();
    return thrift_not_connected(type);
}

// thrift客户端辅助类
//
// 使用示例：
// mooon::net::CThriftClientHelper<ExampleServiceClient> client(rpc_server_ip, rpc_server_port);
// try
// {
//     client.connect();
//     client->foo();
// }
// catch (apache::thrift::transport::TTransportException& transport_ex)
// {
//     MYLOG_ERROR("thrift exception: %s\n", transport_ex.what());
// }
// catch (apache::thrift::transport::TApplicationException& app_ex)
// {
//     MYLOG_ERROR("thrift exception: %s\n", app_ex.what());
// }
// catch (apache::thrift::TException& tx)
// {
//     MYLOG_ERROR("thrift exception: %s\n", tx.what());
// }
// Transport除默认的TFramedTransport (TBufferTransports.h)，还可选择：
// TBufferedTransport (TBufferTransports.h)
// THttpTransport
// TZlibTransport
// TFDTransport (TSimpleFileTransport)
//
// Protocol除默认的apache::thrift::protocol::TBinaryProtocol，还可选择：
// TCompactProtocol
// TJSONProtocol
// TDebugProtocol
template <class ThriftClient,
          class Protocol=apache::thrift::protocol::TBinaryProtocol,
          class Transport=apache::thrift::transport::TFramedTransport>
class CThriftClientHelper
{
public:
    // host thrift服务端的IP地址
    // port thrift服务端的端口号
    // connect_timeout_milliseconds 连接thrift服务端的超时毫秒数
    // receive_timeout_milliseconds 接收thrift服务端发过来的数据的超时毫秒数
    // send_timeout_milliseconds 向thrift服务端发送数据时的超时毫秒数
    CThriftClientHelper(const std::string &host, uint16_t port,
                        int connect_timeout_milliseconds=2000,
                        int receive_timeout_milliseconds=2000,
                        int send_timeout_milliseconds=2000);
    ~CThriftClientHelper();

    // 连接thrift服务端
    //
    // 出错时，可抛出以下几个thrift异常：
    // apache::thrift::transport::TTransportException
    // apache::thrift::TApplicationException
    // apache::thrift::TException
    void connect();

    // 断开与thrift服务端的连接
    //
    // 出错时，可抛出以下几个thrift异常：
    // apache::thrift::transport::TTransportException
    // apache::thrift::TApplicationException
    // apache::thrift::TException
    void close();

    ThriftClient* get() { return _client.get(); }
    ThriftClient* get() const { return _client.get(); }
    ThriftClient* operator ->() { return get(); }
    ThriftClient* operator ->() const { return get(); }

    const std::string& get_host() const { return _host; }
    uint16_t get_port() const { return _port; }

private:
    std::string _host;
    uint16_t _port;
    boost::shared_ptr<apache::thrift::transport::TSocketPool> _sock_pool;
    boost::shared_ptr<apache::thrift::transport::TTransport> _socket;
    boost::shared_ptr<apache::thrift::transport::TFramedTransport> _transport;
    boost::shared_ptr<apache::thrift::protocol::TProtocol> _protocol;
    boost::shared_ptr<ThriftClient> _client;
};

////////////////////////////////////////////////////////////////////////////////
// thrift服务端辅助类
//
// 使用示例：
// mooon::net::CThriftServerHelper<CExampleHandler, ExampleServiceProcessor> _thrift_server;
// try
// {
//     _thrift_server.serve(listen_port);
// }
// catch (apache::thrift::TException& tx)
// {
//     MYLOG_ERROR("thrift exception: %s\n", tx.what());
// }
// ProtocolFactory除了默认的TBinaryProtocolFactory，还可选择：
// TCompactProtocolFactory
// TJSONProtocolFactory
// TDebugProtocolFactory
//
// Server除默认的TNonblockingServer外，还可选择：
// TSimpleServer
// TThreadedServer
// TThreadPoolServer
template <class ThriftHandler,
          class ServiceProcessor,
          class ProtocolFactory=apache::thrift::protocol::TBinaryProtocolFactory,
          class Server=apache::thrift::server::TNonblockingServer>
class CThriftServerHelper
{
public:
    // 启动rpc服务，请注意该调用是同步阻塞的，所以需放最后调用
    // port thrift服务端的监听端口号
    // num_threads thrift服务端开启的线程数
    //
    // 出错时，可抛出以下几个thrift异常：
    // apache::thrift::transport::TTransportException
    // apache::thrift::TApplicationException
    // apache::thrift::TException
    void serve(uint16_t port);
    void serve(uint16_t port, uint8_t num_threads);
    void serve(const std::string &ip, uint16_t port, uint8_t num_threads);
    void stop();

private:
    boost::shared_ptr<ThriftHandler> _handler;
    boost::shared_ptr<apache::thrift::TProcessor> _processor;
    boost::shared_ptr<apache::thrift::protocol::TProtocolFactory> _protocol_factory;
    boost::shared_ptr<apache::thrift::server::ThreadManager> _thread_manager;
    boost::shared_ptr<apache::thrift::concurrency::PosixThreadFactory> _thread_factory;
    boost::shared_ptr<apache::thrift::server::TServer> _server;
};

////////////////////////////////////////////////////////////////////////////////
template <class ThriftClient, class Protocol, class Transport>
CThriftClientHelper<ThriftClient, Protocol, Transport>::CThriftClientHelper(
        const std::string &host, uint16_t port,
        int connect_timeout_milliseconds, int receive_timeout_milliseconds, int send_timeout_milliseconds)
        : _host(host)
        , _port(port)
{
    _sock_pool.reset(new apache::thrift::transport::TSocketPool());
    _sock_pool->addServer(host, (int)port);
    _sock_pool->setConnTimeout(connect_timeout_milliseconds);
    _sock_pool->setRecvTimeout(receive_timeout_milliseconds);
    _sock_pool->setSendTimeout(send_timeout_milliseconds);

    _socket = _sock_pool;
    // Transport默认为apache::thrift::transport::TFramedTransport
    _transport.reset(new Transport(_socket));
    // Protocol默认为apache::thrift::protocol::TBinaryProtocol
    _protocol.reset(new Protocol(_transport));

    _client.reset(new ThriftClient(_protocol));
}

template <class ThriftClient, class Protocol, class Transport>
CThriftClientHelper<ThriftClient, Protocol, Transport>::~CThriftClientHelper()
{
    close();
}

template <class ThriftClient, class Protocol, class Transport>
void CThriftClientHelper<ThriftClient, Protocol, Transport>::connect()
{
    if (!_transport->isOpen())
    {
        _transport->open();
    }
}

template <class ThriftClient, class Protocol, class Transport>
void CThriftClientHelper<ThriftClient, Protocol, Transport>::close()
{
    if (_transport->isOpen())
    {
        _transport->close();
    }
}

////////////////////////////////////////////////////////////////////////////////
template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory, class Server>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory, Server>::serve(uint16_t port)
{
    serve("0.0.0.0", port, 1);
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory, class Server>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory, Server>::serve(uint16_t port, uint8_t num_threads)
{
    serve("0.0.0.0", port, num_threads);
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory, class Server>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory, Server>::serve(const std::string &ip, uint16_t port, uint8_t num_threads)
{
    _handler.reset(new ThriftHandler);
    _processor.reset(new ServiceProcessor(_handler));

    // ProtocolFactory默认为apache::thrift::protocol::TBinaryProtocolFactory
    _protocol_factory.reset(new ProtocolFactory());
    _thread_manager = apache::thrift::server::ThreadManager::newSimpleThreadManager(num_threads);
    _thread_factory.reset(new apache::thrift::concurrency::PosixThreadFactory());

    _thread_manager->threadFactory(_thread_factory);
    _thread_manager->start();

    // Server默认为apache::thrift::server::TNonblockingServer
    _server.reset(new Server(_processor, _protocol_factory, port, _thread_manager));
    _server->run(); // 这里也可直接调用serve()，但推荐run()
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory, class Server>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory, Server>::stop()
{
    _server->stop();
}

NET_NAMESPACE_END
#endif // MOOON_NET_THRIFT_HELPER_H
