// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "db_proxy_handler.h"
#include "sql_logger.h"
#include "DbProxyService.h" // 执行cmake或make db_proxy_rpc时生成的文件
#include <mooon/net/thrift_helper.h>
#include <mooon/observer/observer_manager.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/scoped_ptr.h>

// 服务端口
INTEGER_ARG_DEFINE(uint16_t, port, 4077, 1000, 65535, "listen port of db proxy");

// 日志级别
STRING_ARG_DEFINE(log_level, "info", "set log level: detail, debug, info, error, warn, fatal");
// 是否日志打屏
INTEGER_ARG_DEFINE(uint8_t, screen, 0, 0, 1, "print log on screen if 1");

// 数据上报频率（单位为秒），如果值为0表示禁止收集数据
INTEGER_ARG_DEFINE(uint16_t, report_frequency_seconds, 0, 0, 3600, "frequency seconds to report data");

// 是否记录SQL日志
INTEGER_ARG_DEFINE(uint8_t, log_sql, 0, 0, 1, "write sql to special log file");
// 单个SQL日志文件大小，单位为字节数，默认为500MB，最大为1GB，最小为1KB
INTEGER_ARG_DEFINE(uint32_t, sql_log_filesize, 5242880, 1024, 1073741824, "bytes of a sql log file size");

class CMainHelper: public mooon::sys::IMainHelper
{
public:
    CMainHelper();
    ~CMainHelper();
    void stop(int signo);

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();

private:
    mooon::utils::ScopedPtr<mooon::sys::CSafeLogger> _data_logger;
    mooon::utils::ScopedPtr<mooon::observer::CDefaultDataReporter> _data_reporter;
    mooon::observer::IObserverManager* _observer_manager;
    mooon::net::CThriftServerHelper<
        mooon::db_proxy::CDbProxyHandler, mooon::db_proxy::DbProxyServiceProcessor> _thrift_server;
};

static CMainHelper* g_main_helper = NULL;
static void on_signal(int signo)
{
    g_main_helper->stop(signo);
}

// 参数说明：
// 1) --port rpc服务端口号
//
// 运行示例：
// ./db_proxy --port=8888
extern "C" int main(int argc, char* argv[])
{
    g_main_helper = new CMainHelper;
    mooon::utils::ScopedPtr<CMainHelper> main_helper(g_main_helper);
    return mooon::sys::main_template(main_helper.get(), argc, argv);
}

CMainHelper::CMainHelper()
    : _observer_manager(NULL)
{
}

CMainHelper::~CMainHelper()
{
    mooon::observer::destroy();
}

void CMainHelper::stop(int signo)
{
    _thrift_server.stop();
    mooon::db_proxy::CConfigLoader::get_singleton()->stop_monitor();
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
        mooon::sys::g_logger = mooon::sys::create_safe_logger(true, 8096);
        if (mooon::argument::screen->value() == 1) // 日志打印到屏幕上
            mooon::sys::g_logger->enable_screen(true);
        mooon::sys::log_level_t log_level = mooon::sys::get_log_level(mooon::argument::log_level->c_value());
        mooon::sys::g_logger->set_log_level(log_level);

        // 只有当参数report_frequency_seconds的值大于0时才启动统计功能
        int report_frequency_seconds = mooon::argument::report_frequency_seconds->value();
        if (report_frequency_seconds > 0)
        {
            std::string data_dirpath = mooon::observer::get_data_dirpath();
            if (data_dirpath.empty())
                return false;

            _data_logger.reset(new mooon::sys::CSafeLogger(data_dirpath.c_str(), "db_proxy.data"));
            _data_logger->enable_raw_log(true);
            _data_reporter.reset(new mooon::observer::CDefaultDataReporter(_data_logger.get()));

            _observer_manager = mooon::observer::create(_data_reporter.get(), report_frequency_seconds);
            if (NULL == _observer_manager)
                return false;
        }

        // SQL日志文件大小
        mooon::db_proxy::CSqlLogger::sql_log_filesize = mooon::argument::sql_log_filesize->value();

        // 注册信息，以支持优雅退出不丢数据
        signal(SIGTERM, on_signal);

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
        MYLOG_INFO("db_proxy exit now\n");
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
