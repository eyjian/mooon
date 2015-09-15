// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "db_proxy_handler.h"
#include "rpc/DbProxyService.h" // 执行cmake或make rpc时生成的文件
#include <mooon/net/thrift_helper.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/utils/args_parser.h>

// 服务端口
INTEGER_ARG_DEFINE(uint16_t, port, 8080, 1000, 65535, "listen port of db proxy");
// 是否日志打屏
STRING_ARG_DEFINE(screen, "false", "print log on screen");
// 日志级别
STRING_ARG_DEFINE(log_level, "info", "set log level: detail, debug, info, error, warn, fatal");

class CMainHelper: public mooon::sys::IMainHelper
{
private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();

private:
    mooon::net::CThriftServerHelper<
        mooon::db_proxy::CDbProxyHandler, mooon::db_proxy::DbProxyServiceProcessor> _thrift_server;
};

// 参数说明：
// 1) --port rpc服务端口号
//
// 运行示例：
// ./db_proxy --port=8888
extern "C" int main(int argc, char* argv[])
{
    CMainHelper main_helper;
    return mooon::sys::main_template(&main_helper, argc, argv);
}

bool CMainHelper::init(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return false;
    }

    try
    {
        mooon::sys::g_logger = mooon::sys::create_safe_logger(true, 1024);
        if (mooon::argument::screen->value() == "true")
            mooon::sys::g_logger->enable_screen(true);
        mooon::sys::log_level_t log_level = mooon::sys::get_log_level(mooon::argument::log_level->c_value());
        mooon::sys::g_logger->set_log_level(log_level);

        std::string filepath = mooon::db_proxy::CConfigLoader::get_filepath();
        return mooon::db_proxy::CConfigLoader::get_singleton()->load(filepath);
    }
    catch (mooon::sys::CSyscallException& syscall_ex)
    {
        MYLOG_ERROR("%s\n", syscall_ex.str().c_str());
        return false;
    }
}

bool CMainHelper::run()
{
    mooon::db_proxy::CConfigLoader* config_loader = mooon::db_proxy::CConfigLoader::get_singleton();
    mooon::sys::CThreadEngine monitor(mooon::sys::bind(&mooon::db_proxy::CConfigLoader::monitor, config_loader));

    try
    {
        MYLOG_INFO("thrift will listen on port[%u]\n", mooon::argument::port->value());
        _thrift_server.serve(mooon::argument::port->value());
        return true;
    }
    catch (apache::thrift::TException& tx)
    {
        fprintf(stderr, "thrift exception: %s\n", tx.what());
        return false;
    }
}

void CMainHelper::fini()
{
}
