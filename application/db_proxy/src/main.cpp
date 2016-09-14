// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "db_proxy_handler.h"
#include "sql_logger.h"
#include "DbProxyService.h" // 执行cmake或make db_proxy_rpc时生成的文件
#include <mooon/net/thrift_helper.h>
#include <mooon/observer/observer_manager.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
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

// IO线程数
INTEGER_ARG_DEFINE(uint8_t, num_io_threads, 1, 1, 50, "number of IO threads");
// 工作线程数
INTEGER_ARG_DEFINE(uint8_t, num_work_threads, 1, 1, 50, "number of work threads");

class CMainHelper: public mooon::sys::IMainHelper
{
public:
    CMainHelper();
    ~CMainHelper();

public:
    void signal_thread();
    void on_terminated();
    void on_child_end(pid_t child_pid, int child_exited_status);
    void on_signal_handler(int signo);
    void on_exception(int errcode) throw ();

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();
    virtual std::string get_restart_env_name() const { return std::string("DB_PROXY_AUTO_RESTART"); }

private:
    volatile bool _stop_signal_thread;
    mooon::sys::CThreadEngine* _signal_thread;

private:
    mooon::utils::ScopedPtr<mooon::sys::CSafeLogger> _data_logger;
    mooon::utils::ScopedPtr<mooon::observer::CDefaultDataReporter> _data_reporter;
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
    : _stop_signal_thread(false), _signal_thread(NULL), _observer_manager(NULL)
{
}

CMainHelper::~CMainHelper()
{
    mooon::observer::destroy();
}

void CMainHelper::signal_thread()
{
    mooon::sys::CUtils::set_process_name("db_proxy_sigthread");
    //mooon::sys::CUtils::set_process_title("sig_thread");

    while (!_stop_signal_thread)
    {
        mooon::sys::CSignalHandler::handle(this);
    }
}

void CMainHelper::on_terminated()
{
    _thrift_server.stop();
    mooon::db_proxy::CConfigLoader::get_singleton()->stop_monitor();
    _stop_signal_thread = true;
    MYLOG_INFO("db_proxy will exit by SIGTERM\n");
}

void CMainHelper::on_child_end(pid_t child_pid, int child_exited_status)
{
}

void CMainHelper::on_signal_handler(int signo)
{
    if (SIGINT == signo)
    {
        _thrift_server.stop();
        mooon::db_proxy::CConfigLoader::get_singleton()->stop_monitor();
        _stop_signal_thread = true;
        MYLOG_INFO("db_proxy will exit by SIGINT\n");
    }
}

void CMainHelper::on_exception(int errcode) throw ()
{
    MYLOG_ERROR("(%d)%s\n", errcode, mooon::sys::Error::to_string(errcode).c_str());
}

bool CMainHelper::init(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return false;
    }

    // 阻塞SIGINT、SIG_CHLD和SIG_TERM三个信号
    mooon::sys::CSignalHandler::block_signal(SIGCHLD);
    mooon::sys::CSignalHandler::block_signal(SIGINT);
    mooon::sys::CSignalHandler::block_signal(SIGTERM);

    // 创建信号线程
    _signal_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&CMainHelper::signal_thread, this));
    mooon::sys::CUtils::init_process_title(argc, argv);

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

            const std::string program_short_name = mooon::sys::CUtils::get_program_short_name();
            const std::string data_filename = mooon::utils::CStringUtils::replace_suffix(program_short_name, ".data");
            _data_logger.reset(new mooon::sys::CSafeLogger(data_dirpath.c_str(), data_filename.c_str()));
            _data_logger->enable_raw_log(true);
            _data_reporter.reset(new mooon::observer::CDefaultDataReporter(_data_logger.get()));

            _observer_manager = mooon::observer::create(_data_reporter.get(), report_frequency_seconds);
            if (NULL == _observer_manager)
                return false;
        }

        // SQL日志文件大小
        mooon::db_proxy::CSqlLogger::sql_log_filesize = mooon::argument::sql_log_filesize->value();

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
        MYLOG_INFO("number of IO threads is %d\n", mooon::argument::num_io_threads->value());
        MYLOG_INFO("number of work threads is %d\n", mooon::argument::num_work_threads->value());
        _thrift_server.serve(mooon::argument::port->value(), mooon::argument::num_work_threads->value(), mooon::argument::num_io_threads->value());
        MYLOG_INFO("thrift server exit\n");

        if (_signal_thread != NULL)
        {
            _signal_thread->join();
            MYLOG_INFO("signal thread exit\n");
        }

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
