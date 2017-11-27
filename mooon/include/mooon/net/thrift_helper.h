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
#include <mooon/sys/log.h>
#include <mooon/utils/string_utils.h>
#include <mooon/utils/scoped_ptr.h>
#include <arpa/inet.h>
#include <boost/scoped_ptr.hpp>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/TApplicationException.h>
#include <thrift/transport/TSocketPool.h>
#include <thrift/transport/TTransportException.h>
#include <vector>
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
// catch (apache::thrift::transport::TTransportException& ex)
// {
//     MYLOG_ERROR("thrift transport exception: (%d)%s\n", ex.getType(), ex.what());
// }
// catch (apache::thrift::TApplicationException& ex)
// {
//     MYLOG_ERROR("thrift application exception: %s\n", ex.what());
// }
// catch (apache::thrift::TException& ex)
// {
//     MYLOG_ERROR("thrift exception: %s\n", ex.what());
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
    // set_log_function 是否设置写日志函数，默认设置为debug级别日志
    CThriftClientHelper(const std::string &host, uint16_t port,
                        int connect_timeout_milliseconds=2000,
                        int receive_timeout_milliseconds=2000,
                        int send_timeout_milliseconds=2000,
                        bool set_log_function=true);

    // 支持指定多个servers，运行时随机选择一个，当一个异常时自动选择其它
    // num_retries 重试次数
    // retry_interval 重试间隔，单位为秒
    // max_consecutive_failures 单个Server最大连续失败次数
    // randomize_ 是否随机选择一个Server
    // always_try_last 是否总是重试最后一个Server
    // set_log_function 是否设置写日志函数，默认设置为debug级别日志
    CThriftClientHelper(const std::vector<std::pair<std::string, int> >& servers,
                        int connect_timeout_milliseconds=2000,
                        int receive_timeout_milliseconds=2000,
                        int send_timeout_milliseconds=2000,
                        int num_retries=1, int retry_interval=60,
                        int max_consecutive_failures=1,
                        bool randomize=true, bool always_try_last=true,
                        bool set_log_function=true
                        );
    ~CThriftClientHelper();

    // 连接thrift服务端
    //
    // 出错时，可抛出以下几个thrift异常：
    // apache::thrift::transport::TTransportException
    // apache::thrift::TApplicationException
    // apache::thrift::TException
    void connect();
    bool is_connected() const;

    // 断开与thrift服务端的连接
    //
    // 出错时，可抛出以下几个thrift异常：
    // apache::thrift::transport::TTransportException
    // apache::thrift::TApplicationException
    // apache::thrift::TException
    void close();

    apache::thrift::transport::TSocket* get_socket() { return _socket.get(); }
    const apache::thrift::transport::TSocket get_socket() const { return _socket.get(); }
    ThriftClient* get() { return _client.get(); }
    ThriftClient* get() const { return _client.get(); }
    ThriftClient* operator ->() { return get(); }
    ThriftClient* operator ->() const { return get(); }

    // 取thrift服务端的IP地址
    std::string get_host() const;
    // 取thrift服务端的端口号
    uint16_t get_port() const;

    // 返回可读的标识，常用于记录日志
    std::string str() const
    {
        return utils::CStringUtils::format_string("thrift://%s:%u", get_host().c_str(), get_port());
    }

private:
    void init();

private:
    int _connect_timeout_milliseconds;
    int _receive_timeout_milliseconds;
    int _send_timeout_milliseconds;

private:
    // TSocket只支持一个server，而TSocketPool是TSocket的子类支持指定多个server，运行时随机选择一个
    boost::shared_ptr<apache::thrift::transport::TSocket> _socket;
    boost::shared_ptr<apache::thrift::transport::TTransport> _transport;
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
// catch (apache::thrift::TException& ex)
// {
//     MYLOG_ERROR("thrift exception: %s\n", ex.what());
// }
// ProtocolFactory除了默认的TBinaryProtocolFactory，还可选择：
// TCompactProtocolFactory
// TJSONProtocolFactory
// TDebugProtocolFactory
//
// 只支持TNonblockingServer一种Server
template <class ThriftHandler,
          class ServiceProcessor,
          class ProtocolFactory=apache::thrift::protocol::TBinaryProtocolFactory>
class CThriftServerHelper
{
public:
    // set_log_function 是否设置写日志函数，默认设置为debug级别日志
    CThriftServerHelper(apache::thrift::server::TServerEventHandler* server_event_handler=NULL, bool set_log_function=true);

    // 启动rpc服务，请注意该调用是同步阻塞的，所以需放最后调用
    // port thrift服务端的监听端口号
    // num_threads thrift服务端开启的线程数
    //
    // 出错时，可抛出以下几个thrift异常：
    // apache::thrift::transport::TTransportException
    // apache::thrift::TApplicationException
    // apache::thrift::TException
    // 参数num_io_threads，只有当Server为TNonblockingServer才有效
    void serve(uint16_t port, uint8_t num_worker_threads=1, uint8_t num_io_threads=1);
    void serve(const std::string &ip, uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads=1);
    void serve(const std::string &ip, uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads, void* attached);

    // 要求ThriftHandler类有方法attach(void*)
    void serve(uint16_t port, void* attached, uint8_t num_worker_threads=1, uint8_t num_io_threads=1);

    // 对于TNonblockingServer调用stop时是停止所有的IO线程，做法是设置一个结束循环标志：
    // for (uint32_t i = 0; i < ioThreads_.size(); ++i) ioThreads_[i]->stop();
    // void TNonblockingIOThread::stop() {
    //     // 调用了event_base_loopbreak()
    //     breakLoop(false); // This should cause the thread to fall out of its event loop ASAP.
    // }
    void stop();

    ThriftHandler* get()
    {
        return _handler.get();
    }

    ThriftHandler* get() const
    {
        return _handler.get();
    }

private:
    void init1(apache::thrift::server::TServerEventHandler* server_event_handler, bool set_log_function);
    void init2(const std::string &ip, uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads);

private:
    boost::shared_ptr<apache::thrift::server::TServerEventHandler> _server_event_handler;
    boost::shared_ptr<ThriftHandler> _handler;
    boost::shared_ptr<apache::thrift::TProcessor> _processor;
    boost::shared_ptr<apache::thrift::protocol::TProtocolFactory> _protocol_factory;
    boost::shared_ptr<apache::thrift::server::ThreadManager> _thread_manager;
    boost::shared_ptr<apache::thrift::concurrency::PosixThreadFactory> _thread_factory;
    boost::shared_ptr<apache::thrift::server::TServer> _server;
};

////////////////////////////////////////////////////////////////////////////////
// 被thrift回调的写日志函数，由set_thrift_debug_log_function()调用它
inline void write_thrift_debug_log(const char* log)
{
    if (::mooon::sys::g_logger != NULL)
    {
        MYLOG_DEBUG("%s", log);
    }
}

inline void write_thrift_info_log(const char* log)
{
    if (::mooon::sys::g_logger != NULL)
    {
        MYLOG_INFO("%s", log);
    }
}

inline void write_thrift_error_log(const char* log)
{
    if (::mooon::sys::g_logger != NULL)
    {
        MYLOG_ERROR("%s", log);
    }
}

// 将thrift输出写入到日志文件中
inline void set_thrift_debug_log_function()
{
    apache::thrift::GlobalOutput.setOutputFunction(write_thrift_debug_log);
}

inline void set_thrift_info_log_function()
{
    apache::thrift::GlobalOutput.setOutputFunction(write_thrift_debug_log);
}

inline void set_thrift_error_log_function()
{
    apache::thrift::GlobalOutput.setOutputFunction(write_thrift_debug_log);
}

////////////////////////////////////////////////////////////////////////////////
template <class ThriftClient, class Protocol, class Transport>
CThriftClientHelper<ThriftClient, Protocol, Transport>::CThriftClientHelper(
        const std::string &host, uint16_t port,
        int connect_timeout_milliseconds, int receive_timeout_milliseconds, int send_timeout_milliseconds, bool set_log_function)
        : _connect_timeout_milliseconds(connect_timeout_milliseconds),
          _receive_timeout_milliseconds(receive_timeout_milliseconds),
          _send_timeout_milliseconds(send_timeout_milliseconds)
{
    if (set_log_function)
        set_thrift_debug_log_function();
    _socket.reset(new apache::thrift::transport::TSocket(host, port));
    init();
}

template <class ThriftClient, class Protocol, class Transport>
CThriftClientHelper<ThriftClient, Protocol, Transport>::CThriftClientHelper(
        const std::vector<std::pair<std::string, int> >& servers,
        int connect_timeout_milliseconds,
        int receive_timeout_milliseconds,
        int send_timeout_milliseconds,
        int num_retries, int retry_interval,
        int max_consecutive_failures,
        bool randomize, bool always_try_last, bool set_log_function)
        : _connect_timeout_milliseconds(connect_timeout_milliseconds),
          _receive_timeout_milliseconds(receive_timeout_milliseconds),
          _send_timeout_milliseconds(send_timeout_milliseconds)
{
    if (set_log_function)
        set_thrift_debug_log_function();

    apache::thrift::transport::TSocketPool* socket_pool = new apache::thrift::transport::TSocketPool(servers);
    socket_pool->setNumRetries(num_retries);
    socket_pool->setRetryInterval(retry_interval);
    socket_pool->setMaxConsecutiveFailures(max_consecutive_failures);
    socket_pool->setRandomize(randomize);
    socket_pool->setAlwaysTryLast(always_try_last);
    _socket.reset(socket_pool);
    init();
}

template <class ThriftClient, class Protocol, class Transport>
void CThriftClientHelper<ThriftClient, Protocol, Transport>::init()
{
    _socket->setConnTimeout(_connect_timeout_milliseconds);
    _socket->setRecvTimeout(_receive_timeout_milliseconds);
    _socket->setSendTimeout(_send_timeout_milliseconds);

    // Transport默认为apache::thrift::transport::TFramedTransport
    _transport.reset(new Transport(_socket));
    // Protocol默认为apache::thrift::protocol::TBinaryProtocol
    _protocol.reset(new Protocol(_transport));
    // 服务端的Client
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
        // 如果Transport为TFramedTransport，则实际调用：TFramedTransport::open -> TSocketPool::open
        _transport->open();
        // 当"TSocketPool::open: all connections failed"时，
        // TSocketPool::open就抛出异常TTransportException，异常类型为TTransportException::NOT_OPEN
    }
}

template <class ThriftClient, class Protocol, class Transport>
bool CThriftClientHelper<ThriftClient, Protocol, Transport>::is_connected() const
{
    return _transport->isOpen();
}

template <class ThriftClient, class Protocol, class Transport>
void CThriftClientHelper<ThriftClient, Protocol, Transport>::close()
{
    if (_transport->isOpen())
    {
        _transport->close();
    }
}

template <class ThriftClient, class Protocol, class Transport>
std::string CThriftClientHelper<ThriftClient, Protocol, Transport>::get_host() const
{
    return _socket->getHost();
}

template <class ThriftClient, class Protocol, class Transport>
uint16_t CThriftClientHelper<ThriftClient, Protocol, Transport>::get_port() const
{
    return static_cast<uint16_t>(_socket->getPort());
}

////////////////////////////////////////////////////////////////////////////////
template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::CThriftServerHelper(apache::thrift::server::TServerEventHandler* server_event_handler, bool set_log_function)
{
    init1(server_event_handler, set_log_function);
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::serve(uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads)
{
    serve("0.0.0.0", port, num_worker_threads, num_io_threads);
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::serve(const std::string &ip, uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads)
{
    init2("0.0.0.0", port, num_worker_threads, num_io_threads);

    // 这里也可直接调用serve()，但推荐run()
    // !!!注意调用run()的进程或线程会被阻塞
    _server->run();
    _thread_manager->join();
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::serve(const std::string &ip, uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads, void* attached)
{
    init2(ip, port, num_worker_threads, num_io_threads);

    // 关联
    if (attached != NULL)
        _handler->attach(attached);

    // 这里也可直接调用serve()，但推荐run()
    // !!!注意调用run()的进程或线程会被阻塞
    _server->run();
    _thread_manager->join();
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::serve(uint16_t port, void* attached, uint8_t num_worker_threads, uint8_t num_io_threads)
{
    init2("0.0.0.0", port, num_worker_threads, num_io_threads);

    // 关联
    if (attached != NULL)
        _handler->attach(attached);

    // 这里也可直接调用serve()，但推荐run()
    // !!!注意调用run()的进程或线程会被阻塞
    _server->run();
    _thread_manager->join();
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::stop()
{
    _server->stop();
    _thread_manager->stop();
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::init1(apache::thrift::server::TServerEventHandler* server_event_handler, bool set_log_function)
{
    if (set_log_function)
    {
        set_thrift_debug_log_function();
    }

    _server_event_handler.reset(server_event_handler);
    _handler.reset(new ThriftHandler);
    _processor.reset(new ServiceProcessor(_handler));

    // ProtocolFactory默认为apache::thrift::protocol::TBinaryProtocolFactory
    _protocol_factory.reset(new ProtocolFactory());
}

template <class ThriftHandler, class ServiceProcessor, class ProtocolFactory>
void CThriftServerHelper<ThriftHandler, ServiceProcessor, ProtocolFactory>::init2(const std::string &ip, uint16_t port, uint8_t num_worker_threads, uint8_t num_io_threads)
{
    _thread_manager = apache::thrift::server::ThreadManager::newSimpleThreadManager(num_worker_threads);
    _thread_factory.reset(new apache::thrift::concurrency::PosixThreadFactory());

    _thread_manager->threadFactory(_thread_factory);
    _thread_manager->start();

    apache::thrift::server::TNonblockingServer* server = new apache::thrift::server::TNonblockingServer(_processor, _protocol_factory, port, _thread_manager);
    server->setNumIOThreads(num_io_threads);
    server->setServerEventHandler(_server_event_handler);
    _server.reset(server);

    // 不要调用_server->run()，交给serve()来调用，
    // 因为一旦调用了run()后，调用线程或进程就被阻塞了。
}

NET_NAMESPACE_END
#endif // MOOON_NET_THRIFT_HELPER_H
