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
 * Author: laocai_liu@qq.com or laocailiu@gmail.com
 */

#include <sys/logger.h>
#include <sys/main_template.h>
#include <util/string_util.h>
#include <util/args_parser.h>
#include <server/server.h>
#include "config_impl.h"
#include "pp_packet_handler.h"
#include "statistics_print_thread.h"

#define LOG_FILE_NAME "ppserver.log"

using namespace mooon;

INTEGER_ARG_DEFINE(false, uint16_t, port, 0, 1000, 65535, "ping pong server port")
STRING_ARG_DEFINE(false, ip, "127.0.0.1", "ping pong server ip")

PP_NAMESPACE_BEGIN

sys::ILogger* pp_logger = NULL;

class CFactoryImpl: public server::IFactory
{
private:
    virtual server::IPacketHandler* create_packet_handler(server::IConnection* connection)
    {
        return new CppPakcetHandler(connection);
    }
};

/***
  * 使用main函数模板，需要实现sys::IMainHelper接口
  */
class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper();
    ~CMainHelper();

private:        
    virtual bool init(int argc, char* argv[]);    
    virtual void fini();
    virtual int get_exit_signal() const { return SIGUSR1; }

private:
    CStatisticsPrintThread *_statistics_print;
    sys::CLogger* _logger;
    server::server_t _server;
    CConfigImpl _config_impl;
    CFactoryImpl _factory_impl;
};

/***
  * 
  */
extern "C" int main(int argc, char* argv[])
{
    CMainHelper main_helper;

    return sys::main_template(&main_helper, argc, argv);
}

CMainHelper::CMainHelper()
    :_server(NULL)
{
    _logger = new sys::CLogger;
}

CMainHelper::~CMainHelper()
{
    _logger->destroy();
    fprintf(stderr, "CMainHelper::~CMainHelper\n");
}

bool CMainHelper::init(int argc, char* argv[])
{
    _statistics_print = new CStatisticsPrintThread;
    _statistics_print->start();
    // 解析命令行参数
    if (!ArgsParser::parse(argc, argv))
    {
        fprintf(stderr, "Command parameter error: %s.\n", ArgsParser::g_error_message.c_str());
        return false;
    }

    // 日志文件存放目录
    std::string logdir = sys::CUtil::get_program_path();

    // 创建日志器
    _logger->create(logdir.c_str(), LOG_FILE_NAME);

    // 设置日志器
    server::logger = _logger;
    pp_logger = _logger;

    _config_impl.init(ArgsParser::ip->get_value(), ArgsParser::port->get_value());

    // 创建一个MOOON-server组件实例
    _server = server::create(&_config_impl, &_factory_impl);

    return _server != NULL;
}

void CMainHelper::fini()
{
    if (_server != NULL)
    {
        // 销毁MOOON-server组件实例
        server::destroy(_server);
        _server = NULL;
    }
    // 线程退出引用计数会减1，引用计数为0会自动释放
    _statistics_print->stop();
}

PP_NAMESPACE_END

