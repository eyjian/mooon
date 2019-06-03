// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "db_process.h"
#include "db_proxy_handler.h"
#include "sql_logger.h"
#include "DbProxyService.h" // 执行cmake或make db_proxy_rpc时生成的文件
#include <map>
#include <mooon/net/thrift_helper.h>
#include <mooon/observer/observer_manager.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/file_locker.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/scoped_ptr.h>
#include <string.h>

// 服务端口
INTEGER_ARG_DEFINE(uint16_t, port, 4077, 1000, 65535, "listen port of db proxy");

// IO线程数
INTEGER_ARG_DEFINE(uint8_t, num_io_threads, 1, 1, 50, "number of IO threads");
// 工作线程数
INTEGER_ARG_DEFINE(uint8_t, num_work_threads, 1, 1, 50, "number of work threads");

// sql日志文件大小，建议大小不小于（1024*1024*100），更小的值是为了方便开发时的测试
INTEGER_ARG_DEFINE(int32_t, sql_file_size, (1024*1024*500), (1024*10), std::numeric_limits<int>::max(), "size of single sql log file");
// 多少行刷新一次，如果值为0则由Linux系统控制，lines的值会严重影响性能，值为0时性能较好，值为1时数据最安全，顶多丢一笔数据
INTEGER_ARG_DEFINE(int32_t, lines, 0, 0, 1000000, "flush sql log after N lines");

// 批量提交SQL数
INTEGER_ARG_DEFINE(uint8_t, batch, 1, 1, std::numeric_limits<uint8_t>::max(), "number of batch commit");
// 效率数据定时输出间隔，单位为秒
INTEGER_ARG_DEFINE(uint16_t, efficiency, 60, 2, std::numeric_limits<uint8_t>::max(), "interval to output efficiency (seconds)");

// 缓存多少笔数据
INTEGER_ARG_DEFINE(int32_t, cache_number, 200000, 1, 200000000, "the number of data cached");
// 清理缓存频率，单位为秒
INTEGER_ARG_DEFINE(int32_t, cleanup_frequency, 10, 1, 3600, "the frequency to cleanup the cached data");

// 历史文件保存天数
INTEGER_ARG_DEFINE(uint16_t, history_days, 60, 1, std::numeric_limits<uint16_t>::max(), "days to keep history files");
// 删除过老历史文件时间点
INTEGER_ARG_DEFINE(uint8_t, history_hour, 2, 0, 23, "hour to delete history files");

// 重启入库进入频率，单位为秒
INTEGER_ARG_DEFINE(uint16_t, restart_frequency, 10, 1, std::numeric_limits<uint16_t>::max(), "the frequency (seconds) to restart db process");

// 当父进程不存在时是否自动退出
INTEGER_ARG_DEFINE(uint8_t, auto_exit, 1, 0, 1, "mdbp will exit when it's parent not exist");

// 日志文件备份数，如果值为0表示使用默认的或环境变量MOOON_LOG_BACKUP指定的
INTEGER_ARG_DEFINE(uint16_t, num_logs, 0, 0, std::numeric_limits<uint16_t>::max(), "number of logs backup");

// 数据上报频率（单位为秒），如果值为0表示禁止收集数据
INTEGER_ARG_DEFINE(uint16_t, report_frequency_seconds, 0, 0, 3600, "frequency seconds to report data");

// 是否启动dbprocess
INTEGER_ARG_DEFINE(uint8_t, dbprocess ,1, 0, 1, "whether to start the db process");

// 配置文件，如果不指定则使用默认的
STRING_ARG_DEFINE(conf, "", "the configuration file, e.g., --conf=/tmp/sql.json");

class CMainHelper: public mooon::sys::IMainHelper
{
public:
    CMainHelper();
    ~CMainHelper();

public:
    void cleanup_cache_thread();
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
    bool create_db_processes();
    bool create_db_process(const struct mooon::db_proxy::DbInfo& dbinfo);

private:
    std::map<pid_t, mooon::db_proxy::DbInfo> _db_process_table; // key为入库进程ID
    volatile bool _stop_signal_thread;
    mooon::sys::CThreadEngine* _signal_thread; // 专门处理信号的线程
    mooon::sys::CThreadEngine* _cleanup_cache_thread; // 清理缓存数据的线程

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
    : _stop_signal_thread(false), _signal_thread(NULL), _cleanup_cache_thread(NULL), _observer_manager(NULL)
{
}

CMainHelper::~CMainHelper()
{
    delete _cleanup_cache_thread;
    delete _signal_thread;
    mooon::observer::destroy();
}

void CMainHelper::cleanup_cache_thread()
{
    mooon::sys::CUtils::set_process_name("db_proxy_cleanup");

    while (!_stop_signal_thread)
    {
        mooon::sys::CUtils::millisleep(1000*mooon::argument::cleanup_frequency->value());

        //mooon::db_proxy::CDbProxyHandler* dbproxy_handler = _thrift_server.get();
        if (NULL == _thrift_server.get())
            break;

        _thrift_server.get()->cleanup_cache();
    }

    MYLOG_INFO("cleanup cache thread exiting\n");
}

void CMainHelper::signal_thread()
{
    mooon::sys::CUtils::set_process_name("db_proxy_signal");

    while (!_stop_signal_thread)
    {
        mooon::sys::CSignalHandler::handle(this);
    }

    MYLOG_INFO("signal thread exiting\n");
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
        const mooon::db_proxy::DbInfo dbinfo = iter->second;

        MYLOG_INFO("db process(%u) exit with code(%d): %s\n", static_cast<unsigned int>(child_pid), child_exited_status, dbinfo.str().c_str());
        _db_process_table.erase(iter);

        if (WIFSTOPPED(child_exited_status))
        {
            const int child_exit_code = WSTOPSIG(child_exited_status);
            MYLOG_INFO("db process(%u) exit by %d\n", child_pid, child_exit_code);
        }
        else if (WIFEXITED(child_exited_status))
        {
            const int child_exit_code = WEXITSTATUS(child_exited_status);
            MYLOG_INFO("db process(%u) exit with %d\n", child_pid, child_exit_code);
        }
        else if (WIFSIGNALED(child_exited_status))
        {
            const int signo = WTERMSIG(child_exited_status);
            MYLOG_INFO("db process(%u) exit by signal: (%d)%s\n", child_pid, signo, strsignal(signo));

            // 异常退出才重启，而且需要控制重启频率
            if ((signo != SIGINT) && (signo != SIGTERM))
            {
                if (mooon::argument::dbprocess->value() != 1)
                {
                    MYLOG_INFO("not restart db process\n");
                }
                else
                {
                    const uint32_t restart_frequency = mooon::argument::restart_frequency->value();
                    MYLOG_INFO("to restart db process(%u) after %us: %s\n", child_pid, restart_frequency, dbinfo.str().c_str());
                    mooon::sys::CUtils::millisleep(1000*restart_frequency);
                    (void)create_db_process(dbinfo);
                }
            }
        }
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
    if (!mooon::sys::CMySQLConnection::library_init())
    {
        fprintf(stderr, "init mysql failed\n");
        return 1;
    }

    // 阻塞SIG_CHLD和SIG_TERM两个信号
    mooon::sys::CSignalHandler::block_signal(SIGCHLD);
    //mooon::sys::CSignalHandler::block_signal(SIGINT);
    mooon::sys::CSignalHandler::block_signal(SIGTERM);

    // 延后1秒，让之前的进程有足够时间完成收尾退出
    mooon::sys::CUtils::millisleep(1000);
    mooon::sys::CUtils::init_process_title(argc, argv);

    // 创建信号线程
    _signal_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&CMainHelper::signal_thread, this));
    // 创建清理缓存线程
    _cleanup_cache_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&CMainHelper::cleanup_cache_thread, this));

    try
    {
        // 日志文件名加上端口作为后缀，这样同一份即可以启动多端口服务
        const uint16_t num_logs = mooon::argument::num_logs->value();
        const uint16_t port = mooon::argument::port->value();
        const std::string port_str = mooon::utils::CStringUtils::int_tostring(port);
        mooon::sys::g_logger = mooon::sys::create_safe_logger(true, 8096, port_str);
        if (num_logs > 0)
        {
            mooon::sys::g_logger->set_backup_number(num_logs);
        }

        // 只有当参数report_frequency_seconds的值大于0时才启动统计功能
        const int report_frequency_seconds = mooon::argument::report_frequency_seconds->value();
        MYLOG_INFO("report_frequency_seconds: %d\n", report_frequency_seconds);
        if (report_frequency_seconds > 0)
        {
            mooon::observer::observer_logger = mooon::sys::g_logger;

            std::string data_dirpath = mooon::observer::get_data_dirpath();
            if (data_dirpath.empty())
                return false;

            _data_logger.reset(new mooon::sys::CSafeLogger(data_dirpath.c_str(), mooon::utils::CStringUtils::format_string("db_proxy_%d.data", mooon::argument::port->value()).c_str()));
            _data_logger->enable_raw_log(true);
            _data_logger->set_backup_number(2);
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

        return create_db_processes();
    }
    catch (mooon::sys::CSyscallException& syscall_ex)
    {
        MYLOG_ERROR("%s\n", syscall_ex.str().c_str());
        return false;
    }
}

bool CMainHelper::run()
{
    try
    {
        MYLOG_INFO("thrift will listen on port[%u]\n", mooon::argument::port->value());
        MYLOG_INFO("number of IO threads is %d\n", mooon::argument::num_io_threads->value());
        MYLOG_INFO("number of work threads is %d\n", mooon::argument::num_work_threads->value());
        _thrift_server.serve(mooon::argument::port->value(), mooon::argument::num_work_threads->value(), mooon::argument::num_io_threads->value());
        MYLOG_INFO("thrift server exit\n");

        if (_cleanup_cache_thread != NULL)
        {
            _cleanup_cache_thread->join();
            MYLOG_INFO("cleanup cache thread exit\n");
        }
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
    mooon::sys::CMySQLConnection::library_end();
}

void CMainHelper::stop()
{
    _thrift_server.stop();
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

bool CMainHelper::create_db_processes()
{
    if (mooon::argument::dbprocess->value() != 1)
    {
        MYLOG_INFO("not start db process\n");
        return true;
    }

    for (int index=0; index<mooon::db_proxy::MAX_DB_CONNECTION; ++index)
    {
        struct mooon::db_proxy::DbInfo dbinfo;
        if (mooon::db_proxy::CConfigLoader::get_singleton()->get_db_info(index, &dbinfo))
        {
            if (!create_db_process(dbinfo))
                return false;
        }
    }

    return true;
}

bool CMainHelper::create_db_process(const struct mooon::db_proxy::DbInfo& dbinfo)
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

    return true;
}
