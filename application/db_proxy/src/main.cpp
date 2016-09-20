// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "db_process.h"
#include "db_proxy_handler.h"
#include "sql_logger.h"
#include "DbProxyService.h" // 执行cmake或make db_proxy_rpc时生成的文件
#include <map>
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

// 数据上报频率（单位为秒），如果值为0表示禁止收集数据
INTEGER_ARG_DEFINE(uint16_t, report_frequency_seconds, 0, 0, 3600, "frequency seconds to report data");

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
    void stop();
    bool create_db_process();

private:
    std::map<pid_t, mooon::db_proxy::DbInfo> _db_process_table; // key为入库进程ID
    volatile bool _stop_signal_thread;
    mooon::sys::CThreadEngine* _signal_thread;

private:
    mooon::utils::ScopedPtr<mooon::sys::CSafeLogger> _data_logger;
    mooon::utils::ScopedPtr<mooon::observer::CDefaultDataReporter> _data_reporter;
    mooon::observer::IObserverManager* _observer_manager;
    mooon::net::CThriftServerHelper<mooon::db_proxy::CDbProxyHandler, mooon::db_proxy::DbProxyServiceProcessor> _thrift_server;
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
    stop();
    MYLOG_INFO("db_proxy will exit by SIGTERM\n");
}

void CMainHelper::on_child_end(pid_t child_pid, int child_exited_status)
{
    std::map<pid_t, mooon::db_proxy::DbInfo>::iterator iter = _db_process_table.find(child_pid);
    if (iter == _db_process_table.end())
    {
        MYLOG_WARN("not found db process(%u), exited with code(%d)\n", static_cast<unsigned int>(child_pid), child_exited_status);
    }
    else
    {
        MYLOG_INFO("db process(%u) exit with code(%d)\n", static_cast<unsigned int>(child_pid), child_exited_status);
        _db_process_table.erase(iter);
    }
}

void CMainHelper::on_signal_handler(int signo)
{
    if (SIGINT == signo)
    {
        stop();
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

    // 阻塞SIG_CHLD和SIG_TERM两个信号
    mooon::sys::CSignalHandler::block_signal(SIGCHLD);
    //mooon::sys::CSignalHandler::block_signal(SIGINT);
    mooon::sys::CSignalHandler::block_signal(SIGTERM);

    // 创建信号线程
    mooon::sys::CUtils::millisleep(1000);
    _signal_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&CMainHelper::signal_thread, this));
    mooon::sys::CUtils::init_process_title(argc, argv);

    try
    {
        mooon::sys::g_logger = mooon::sys::create_safe_logger(true, 8096);

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

        std::string filepath = mooon::db_proxy::CConfigLoader::get_filepath();
        if (!mooon::db_proxy::CConfigLoader::get_singleton()->load(filepath))
        {
            return false;
        }

        return create_db_process();
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

void CMainHelper::stop()
{
    _thrift_server.stop();
    mooon::db_proxy::CConfigLoader::get_singleton()->stop_monitor();
    _stop_signal_thread = true;

    for (std::map<pid_t, mooon::db_proxy::DbInfo>::iterator iter=_db_process_table.begin(); iter!=_db_process_table.end(); ++iter)
    {
        const pid_t db_pid = iter->first;
        const mooon::db_proxy::DbInfo& dbinfo = iter->second;

        MYLOG_INFO("tell dbprocess(%u, %s) to exit\n", static_cast<unsigned int>(db_pid), dbinfo.str().c_str());
        kill(db_pid, SIGTERM); // Tell db process to exit.
    }

    // 让db_proxy尽量忙完
    mooon::sys::CUtils::millisleep(200);
}

bool CMainHelper::create_db_process()
{
    for (int index=0; index<mooon::db_proxy::MAX_DB_CONNECTION; ++index)
    {
        struct mooon::db_proxy::DbInfo dbinfo;
        if (mooon::db_proxy::CConfigLoader::get_singleton()->get_db_info(index, &dbinfo))
        {
            if (dbinfo.alias.empty())
            {
                MYLOG_INFO("alias empty, no dbprocess(%s)\n", dbinfo.str().c_str());
            }
            else
            {
                pid_t db_pid = fork();
                if (-1 == db_pid)
                {
                    MYLOG_ERROR("create dbprocess(%s) error: %s\n", dbinfo.str().c_str(), mooon::sys::Error::to_string().c_str());
                    return false;
                }
                else if (0 == db_pid)
                {
                    // 入库进程
                    mooon::db_proxy::CDbProcess db_process(dbinfo);
                    db_process.run();
                    MYLOG_INFO("dbprocess(%u, %s) exit\n", static_cast<unsigned int>(getpid()), dbinfo.str().c_str());
                    exit(0);
                }
                else
                {
                    _db_process_table.insert(std::make_pair(db_pid, dbinfo));
                    MYLOG_INFO("add dbprocess(%u, %s)\n", static_cast<unsigned int>(db_pid), dbinfo.str().c_str());
                }
            }
        }
    }

    return true;
}
