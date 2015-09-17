// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "data_reporter.h"
#include "db_proxy_handler.h"
#include "rpc/DbProxyService.h" // 执行cmake或make rpc时生成的文件
#include <mooon/net/thrift_helper.h>
#include <mooon/observer/observer_manager.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/scoped_ptr.h>

// 服务端口
INTEGER_ARG_DEFINE(uint16_t, port, 4077, 1000, 65535, "listen port of db proxy");
// 是否日志打屏
STRING_ARG_DEFINE(screen, "false", "print log on screen");
// 日志级别
STRING_ARG_DEFINE(log_level, "info", "set log level: detail, debug, info, error, warn, fatal");
// 状态数据上报频率，单位为秒
INTEGER_ARG_DEFINE(uint16_t, report_frequency_seconds, 0, 0, 3600, "frequency seconds to report data");

class CMainHelper: public mooon::sys::IMainHelper
{
public:
    CMainHelper();
    ~CMainHelper();

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();

private:
    std::string get_data_dirpath() const;

private:
    mooon::utils::ScopedPtr<mooon::sys::CSafeLogger> _data_logger;
    mooon::utils::ScopedPtr<mooon::db_proxy::CDataReporter> _data_reporter;
    mooon::observer::IObserverManager* _observer_manager;
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

CMainHelper::CMainHelper()
    : _observer_manager(NULL)
{
}

CMainHelper::~CMainHelper()
{
    mooon::observer::destroy();
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

        // 只有当参数report_frequency_seconds的值大于0时才启动统计功能
        int report_frequency_seconds = mooon::argument::report_frequency_seconds->value();
        if (report_frequency_seconds > 0)
        {
            std::string data_dirpath = get_data_dirpath();
            if (data_dirpath.empty())
                return false;

            _data_logger.reset(new mooon::sys::CSafeLogger(data_dirpath.c_str(), "db_proxy.data"));
            _data_reporter.reset(new mooon::db_proxy::CDataReporter(_data_logger.get()));

            _observer_manager = mooon::observer::create(_data_reporter.get(), report_frequency_seconds);
            if (NULL == _observer_manager)
                return false;
        }

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

std::string CMainHelper::get_data_dirpath() const
{
    std::string program_path = mooon::sys::CUtils::get_program_path();
    std::string data_dirpath = program_path + std::string("/../data");

    try
    {
        if (mooon::sys::CDirUtils::exist(data_dirpath))
        {
            return data_dirpath;
        }
        else
        {
            MYLOG_ERROR("datadir[%s] not exist\n", data_dirpath.c_str());
        }
    }
    catch (mooon::sys::CSyscallException& syscall_ex)
    {
        MYLOG_ERROR("%s\n", syscall_ex.str().c_str());
    }

    return std::string("");
}
