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
#include <util/args_parser.h>
#include <net/util.h>
#include <dispatcher/dispatcher.h>
#include "pp_director.h"

#define LOG_FILE_NAME "ppclient.log"

using namespace mooon;

INTEGER_ARG_DEFINE(false, uint16_t, port, 0, 1000, 65535, "ping pong server port")
STRING_ARG_DEFINE(false, ip, "127.0.0.1", "ping pong server ip")
INTEGER_ARG_DEFINE(false, uint16_t, thread_count, 1, 1, 2048, "thread count")
INTEGER_ARG_DEFINE(false, int, sender_count, 0, 1, 20480, "sender count")
INTEGER_ARG_DEFINE(false, int, bytes_per_send, 0, 0, 2048, "msg bytes one send")

PP_NAMESPACE_BEGIN

static CppDirector director;

sys::ILogger* pp_logger = NULL;

class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper();
    ~CMainHelper();

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual int get_exit_signal() const { return SIGUSR1; }
    virtual void fini();

private:
    sys::CLogger* _logger;
    dispatcher::IDispatcher* _dispatcher;
};

CMainHelper::CMainHelper()
    :_dispatcher(NULL)
{
    _logger = new sys::CLogger;
}

CMainHelper::~CMainHelper()
{
    _logger->destroy();
}

bool CMainHelper::init(int argc, char* argv[])
{
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
    dispatcher::logger = _logger;
    pp_logger = _logger;

    PP_LOG_INFO("server host %s:%d\n", ArgsParser::ip->get_value().c_str(), ArgsParser::port->get_value());
    PP_LOG_INFO("client thread_count=%d, sender_count=%d, bytes_per_send=%d\n", 
	ArgsParser::thread_count->get_value(), ArgsParser::sender_count->get_value(), ArgsParser::bytes_per_send->get_value());

    // 创建MOOON-dispatcher组件实例
    _dispatcher = dispatcher::create(ArgsParser::thread_count->get_value());
    if (NULL == _dispatcher)
    {
        return false;
    }

    uint32_t int_ip;
    net::CUtil::string_toipv4(ArgsParser::ip->get_value().c_str(), int_ip);
    director.set_server_ip_port(int_ip, ArgsParser::port->get_value());
    director.set_sender_count(ArgsParser::sender_count->get_value());
    director.set_bytes_per_send(ArgsParser::bytes_per_send->get_value());
    director.set_dispatcher(_dispatcher);

    return true;
}

bool CMainHelper::run()
{
    return director.start();
}

void CMainHelper::fini()
{
    if (_dispatcher != NULL)
    {
        // 销毁MOOON-dispatcher组件实例
        dispatcher::destroy(_dispatcher);
        _dispatcher = NULL;
    }

    fprintf(stdout, "CMainHelper fini exit\n");
}

extern "C" int main(int argc, char* argv[])
{
    CMainHelper main_helper;
    return sys::main_template(&main_helper, argc, argv);
}

PP_NAMESPACE_END
