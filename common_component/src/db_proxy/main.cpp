// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "db_proxy_handler.h"
#include "rpc/DbProxyService.h"
#include <mooon/net/thrift_helper.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/utils/args_parser.h>

// 服务端口
INTEGER_ARG_DEFINE(true, uint16_t, port, 8080, 1000, 65535, listen port of db proxy);

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

extern "C" int main(int argc, char* argv[])
{
    CMainHelper main_helper;
    return mooon::sys::main_template(&main_helper, argc, argv);
}

bool CMainHelper::init(int argc, char* argv[])
{
    if (!ArgsParser::parse(argc, argv))
    {
        fprintf(stderr, "%s\n", ArgsParser::g_error_message.c_str());
        return false;
    }

    try
    {
        mooon::sys::g_logger = mooon::sys::create_safe_logger(true, 1024);
        mooon::sys::g_logger->enable_screen(true);

        std::string filepath = mooon::db_proxy::CConfigLoader::get_filepath();
        return mooon::db_proxy::CConfigLoader::get_singleton()->load(filepath);
    }
    catch (mooon::sys::CSyscallException& syscall_ex)
    {
        fprintf(stderr, "%s\n", syscall_ex.what());
        return false;
    }
}

bool CMainHelper::run()
{
    try
    {
        MYLOG_INFO("thrift will listen on port[%u]\n", ArgsParser::port->get_value());
        _thrift_server.serve(ArgsParser::port->get_value());
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
