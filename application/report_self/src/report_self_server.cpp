// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
// 单线程ThriftServer，不支持多线程
#include "ReportSelfService.h"
#include <mooon/net/thrift_helper.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/mmap.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/md5_helper.h>
#include <mooon/utils/string_utils.h>

// 服务端口
INTEGER_ARG_DEFINE(uint16_t, port, 7110, 1000, 65535, "listen port of db proxy");

// DB配置
STRING_ARG_DEFINE(db_ip, "", "database ip");
INTEGER_ARG_DEFINE(uint16_t, db_port, 3306, 1, 65535, "database port");
STRING_ARG_DEFINE(db_user, "", "database user");
STRING_ARG_DEFINE(db_password, "", "database password of user");
STRING_ARG_DEFINE(db_name, "", "database name");

namespace mooon {

static sys::CMySQLConnection g_mysql;

class CReportSelfHandler: public ReportSelfServiceIf
{
private:
    virtual void report(const std::vector<std::string>& tokens);

private:
    std::string make_insert_sql(const std::vector<std::string>& tokens) const;
    std::string make_update_sql(const std::vector<std::string>& tokens) const;
};

class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper();

private:
    virtual bool init(int argc, char* argv[]);
    virtual bool run();
    virtual void fini();

public:
    void signal_thread();
    void on_terminated();
    void on_child_end(pid_t child_pid, int child_exited_status);
    void on_signal_handler(int signo);
    void on_exception(int errcode);

private:
    bool init_mysql();

private:
    volatile bool _stop;
    sys::CThreadEngine* _signal_thread;
    net::CThriftServerHelper<CReportSelfHandler, ReportSelfServiceProcessor> _thrift_server;
};

extern "C" int main(int argc, char* argv[])
{
    CMainHelper main_helper;
    return sys::main_template(&main_helper, argc, argv);
}

CMainHelper::CMainHelper()
    : _stop(false), _signal_thread(NULL)
{
}

bool CMainHelper::init(int argc, char* argv[])
{
    std::string errmsg;
    if (!utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        fprintf(stderr, "%s\n", utils::g_help_string.c_str());
        return false;
    }

    // db_ip
    if (argument::db_ip->value().empty())
    {
        fprintf(stderr, "parameter[--db_ip] not set\n");
        fprintf(stderr, "%s\n", utils::g_help_string.c_str());
        return false;
    }
    // db_user
    if (argument::db_user->value().empty())
    {
        fprintf(stderr, "parameter[--db_user] not set\n");
        fprintf(stderr, "%s\n", utils::g_help_string.c_str());
        return false;
    }
    // db_password
    if (argument::db_password->value().empty())
    {
        fprintf(stderr, "parameter[--db_password] not set\n");
        fprintf(stderr, "%s\n", utils::g_help_string.c_str());
        return false;
    }
    // db_name
    if (argument::db_name->value().empty())
    {
        fprintf(stderr, "parameter[--db_name] not set\n");
        fprintf(stderr, "%s\n", utils::g_help_string.c_str());
        return false;
    }

    try
    {
        sys::g_logger = sys::create_safe_logger();
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "create logger failed: %s\n", ex.str().c_str());
        return false;
    }

    try
    {
        // 通过SIGTERM幽雅退出
        sys::CSignalHandler::block_signal(SIGTERM);
        // 创建信号线程
        _signal_thread = new sys::CThreadEngine(sys::bind(&CMainHelper::signal_thread, this));

        return init_mysql();
    }
    catch (sys::CSyscallException& ex)
    {
        MYLOG_ERROR("%s\n", ex.str().c_str());
        return false;
    }
}

bool CMainHelper::run()
{
    try
    {
        // 一个线程足够了
        const uint8_t num_worker_threads = 1;
        const uint8_t num_io_threads = 1;
        MYLOG_INFO("thrift will listen on port[%u]\n", argument::port->value());
        _thrift_server.serve(argument::port->value(), num_worker_threads, num_io_threads);
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
    if (!_stop && _signal_thread!=NULL)
    {
        // 不能使用raise(SIGTERM)，因为它是针对线程的
        kill(getpid(), SIGTERM);
    }
    if (_signal_thread != NULL)
    {
        _signal_thread->join();
    }

    sys::CMySQLConnection::library_end();
    MYLOG_INFO("exit now\n");
}

void CMainHelper::signal_thread()
{
    while (!_stop)
    {
        sys::CSignalHandler::handle(this);
    }

    MYLOG_INFO("signal thread exit\n");
}

void CMainHelper::on_terminated()
{
    // 优雅退出
    MYLOG_INFO("will exit by SIGTERM\n");
    _stop = true;
}

void CMainHelper::on_child_end(pid_t child_pid, int child_exited_status)
{
    // 不存在子进程
}

void CMainHelper::on_signal_handler(int signo)
{
}

void CMainHelper::on_exception(int errcode)
{
    MYLOG_ERROR("(%d)%s\n", errcode, sys::Error::to_string(errcode).c_str());
}

bool CMainHelper::init_mysql()
{
    try
    {
        g_mysql.set_host(argument::db_ip->value(), argument::db_port->value());
        g_mysql.set_user(argument::db_user->value(), argument::db_password->value());
        g_mysql.set_db_name(argument::db_name->value());
        g_mysql.enable_auto_reconnect(true);
        g_mysql.open();

        MYLOG_INFO("open %s ok\n", g_mysql.str().c_str());
        return true;
    }
    catch (sys::CDBException& ex)
    {
        MYLOG_ERROR("[%s] %s\n", g_mysql.str().c_str(), ex.str().c_str());
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
void CReportSelfHandler::report(const std::vector<std::string>& tokens)
{
    const std::string& update_sql = make_update_sql(tokens);

    if (!update_sql.empty())
    {
        try
        {
            const int n = g_mysql.update("%s", update_sql.c_str());
            MYLOG_INFO("(%d)%s\n", n, update_sql.c_str());

            if (0 == n)
            {
                const std::string& insert_sql = make_insert_sql(tokens);
                if (!insert_sql.empty())
                {
                    const int m = g_mysql.update("%s", insert_sql.c_str());
                    MYLOG_INFO("(%d)%s\n", m, insert_sql.c_str());
                }
            }
        }
        catch (sys::CDBException& ex)
        {
            MYLOG_ERROR("[%s][%s] %s\n", g_mysql.str().c_str(), update_sql.c_str(), ex.str().c_str());

            try
            {
                g_mysql.reopen();
            }
            catch (sys::CDBException& ex)
            {
                MYLOG_ERROR("[%s][%s] %s\n", g_mysql.str().c_str(), update_sql.c_str(), ex.str().c_str());
            }
        }
    }
}

/*
 * 表table_name的结构（表名为t_program_deployment）：
 * f_md5,f_ip,f_user,f_shortname,f_dirpath,f_full_cmdline,f_lasttime,f_interval,f_pid,f_vsz,f_rss,f_firsttime
 *
DROP TABLE IF EXISTS t_program_deployment;
CREATE TABLE t_program_deployment (
    f_id BIGINT NOT NULL AUTO_INCREMENT, # 自增ID
    f_md5 VARCHAR(32) NOT NULL PRIMARY KEY, # f_full_cmdline的MD5值
    f_ip VARCHAR(24) NOT NULL, # 进程所在机器的IP
    f_user VARCHAR(16) NOT NULL, # 进程的当前系统用户名
    f_shortname VARCHAR(64) NOT NULL, # 进程的短名称
    f_dirpath VARCHAR(256) NOT NULL, # 程序所在目录
    f_full_cmdline VARCHAR(8192) NOT NULL, # 进程的完全命令行
    f_firsttime DATETIME NOT NULL, # 最近一次上报时间
    f_lasttime DATETIME NOT NULL, # 最近一次上报时间
    f_interval INT UNSIGNED NOT NULL DEFAULT 0, # 上报间隔，单位为秒
    f_pid INT UNSIGNED NOT NULL DEFAULT 0, # 进程的进程ID
    f_vsz BIGINT UNSIGNED NOT NULL DEFAULT 0, # 进程所占的虚拟内存
    f_rss BIGINT UNSIGNED NOT NULL DEFAULT 0, # 进程所占的物理内存
    f_state TINYINT DEFAULT 0,
    f_memo VARCHAR(256),
    INDEX idx_id (f_id),
    INDEX idx_ip (f_ip),
    INDEX idx_pid (f_pid),
    INDEX idx_shortname (f_shortname),
    INDEX idx_firsttime (f_firsttime),
    INDEX idx_lasttime (f_lasttime),
    INDEX idx_state (f_state)
);
 */

std::string CReportSelfHandler::make_insert_sql(const std::vector<std::string>& tokens) const
{
    std::string sql;

    if (tokens.size() < 11)
    {
        MYLOG_ERROR("tokens number error: %zd\n", tokens.size());
    }
    else
    {
        const std::string& full_cmdline_md5 = utils::CStringUtils::trim(tokens[0]);
        const std::string& ip = utils::CStringUtils::trim(tokens[1]);
        const std::string& user = utils::CStringUtils::trim(tokens[2]);
        const std::string& shortname = utils::CStringUtils::trim(tokens[3]);
        const std::string& dirpath = utils::CStringUtils::trim(tokens[4]);
        const std::string& full_cmdline = utils::CStringUtils::trim(tokens[5]);
        const std::string& lasttime = utils::CStringUtils::trim(tokens[6]);
        const std::string& report_interval_seconds = utils::CStringUtils::trim(tokens[7]);
        const std::string& pid = utils::CStringUtils::trim(tokens[8]);
        const std::string& vsz = utils::CStringUtils::trim(tokens[9]);
        const std::string& rss = utils::CStringUtils::trim(tokens[10]);
        const std::string& firsttime = lasttime;
        const std::string& full_cmdline_md5_ = utils::CMd5Helper::md5("%s", full_cmdline.c_str());

        // 检查关键字段
        if (full_cmdline_md5_ != full_cmdline_md5)
        {
            MYLOG_ERROR("invalid tokens: md5 check error\n");
        }
        else if (full_cmdline_md5.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `md5`\n");
        }
        else if (ip.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `ip`\n");
        }
        else if (shortname.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `shortname`\n");
        }
        else if (dirpath.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `dirpath`\n");
        }
        else if (lasttime.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `lasttime`\n");
        }
        else
        {
            sql = utils::CStringUtils::format_string(
                    "INSERT INTO t_program_deployment ("
                    "f_md5,f_ip,f_user,f_shortname,f_dirpath,f_full_cmdline,f_lasttime,f_interval,f_pid,f_vsz,f_rss,f_firsttime) "
                    "VALUES ('%s','%s','%s','%s','%s','%s','%s',%s,%s,%s,%s,'%s')",
                    full_cmdline_md5.c_str(), ip.c_str(), user.c_str(), shortname.c_str(), dirpath.c_str(),
                    full_cmdline.c_str(), lasttime.c_str(), report_interval_seconds.c_str(),
                    pid.c_str(), vsz.c_str(), rss.c_str(), firsttime.c_str());
        }
    }

    return sql;
}

std::string CReportSelfHandler::make_update_sql(const std::vector<std::string>& tokens) const
{
    std::string sql;

    if (tokens.size() < 11)
    {
        MYLOG_ERROR("tokens number error: %zd\n", tokens.size());
    }
    else
    {
        const std::string& full_cmdline_md5 = utils::CStringUtils::trim(tokens[0]);
        const std::string& ip = utils::CStringUtils::trim(tokens[1]);
        const std::string& user = utils::CStringUtils::trim(tokens[2]);
        const std::string& shortname = utils::CStringUtils::trim(tokens[3]);
        const std::string& dirpath = utils::CStringUtils::trim(tokens[4]);
        const std::string& full_cmdline = utils::CStringUtils::trim(tokens[5]);
        const std::string& lasttime = utils::CStringUtils::trim(tokens[6]);
        const std::string& report_interval_seconds = utils::CStringUtils::trim(tokens[7]);
        const std::string& pid = utils::CStringUtils::trim(tokens[8]);
        const std::string& vsz = utils::CStringUtils::trim(tokens[9]);
        const std::string& rss = utils::CStringUtils::trim(tokens[10]);
        const std::string& full_cmdline_md5_ = utils::CMd5Helper::md5("%s", full_cmdline.c_str());

        // 检查关键字段
        if (full_cmdline_md5_ != full_cmdline_md5)
        {
            MYLOG_ERROR("invalid tokens: md5 check error\n");
        }
        else if (full_cmdline_md5.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `md5`\n");
        }
        else if (ip.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `ip`\n");
        }
        else if (shortname.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `shortname`\n");
        }
        else if (dirpath.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `dirpath`\n");
        }
        else if (lasttime.empty())
        {
            MYLOG_ERROR("invalid tokens: empty `lasttime`\n");
        }
        else
        {
            sql = utils::CStringUtils::format_string(
                    "UPDATE t_program_deployment SET "
                    "f_ip='%s',f_user='%s',f_shortname='%s',f_dirpath='%s',f_full_cmdline='%s',f_lasttime='%s',f_interval=%s,f_pid=%s,f_vsz=%s,f_rss=%s "
                    "WHERE f_md5='%s'",
                    ip.c_str(), user.c_str(), shortname.c_str(), dirpath.c_str(),
                    full_cmdline.c_str(), lasttime.c_str(), report_interval_seconds.c_str(),
                    pid.c_str(), vsz.c_str(), rss.c_str(),
                    full_cmdline_md5.c_str());
        }
    }

    return sql;
}

} // namespace mooon {
