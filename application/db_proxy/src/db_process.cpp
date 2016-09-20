// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "db_process.h"
#include <mooon/sys/log.h>
#include <mooon/sys/signal_handler.h>
#include <string.h>
namespace mooon { namespace db_proxy {

CDbProcess::CDbProcess(const struct DbInfo& dbinfo)
    : _dbinfo(dbinfo), _stop_signal_thread(false), _signal_thread(NULL),
      _consecutive_failures(0), _db_connected(false)
{
}

CDbProcess::~CDbProcess()
{
    if (_signal_thread != NULL)
    {
        _signal_thread->join();
        delete _signal_thread;
    }
}

void CDbProcess::run()
{
    sys::CUtils::set_process_title("db_process");

    _signal_thread = new sys::CThreadEngine(sys::bind(&CDbProcess::signal_thread, this));
    while (!_stop_signal_thread)
    {
        pid_t ppid = getppid();
        if (1 == ppid)
        {
            // 父进程不在则自动退出
            MYLOG_INFO("dbprocess(%u, %s) will exit for parent process not exit\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str());
            break;
        }
        if (!_db_connected && !connect_db())
        {
            mooon::sys::CUtils::millisleep(1000);
            continue;
        }

        mooon::sys::CUtils::millisleep(100);
    }

    MYLOG_INFO("dbprocess(%u, %s) exit now\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str());
}

void CDbProcess::signal_thread()
{
    while (!_stop_signal_thread)
    {
        sys::CSignalHandler::handle(this);
    }
}

void CDbProcess::on_terminated()
{
    _stop_signal_thread = true;
    MYLOG_INFO("dbprocess(%u, %s) will exit\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str());
}

void CDbProcess::on_child_end(pid_t child_pid, int child_exited_status)
{
    MYLOG_INFO("dbprocess(%u, %s) receive SIGCHLD with status: %d\n", static_cast<unsigned int>(child_pid), _dbinfo.str().c_str(), child_exited_status);
}

void CDbProcess::on_signal_handler(int signo)
{
    MYLOG_INFO("db process(%u, %s) receive signal: (%d)%s\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), signo, strsignal(signo));
}

void CDbProcess::on_exception(int errcode) throw ()
{
    MYLOG_ERROR("dbprocess(%u, %s) error: (%d)%s\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), errcode, sys::Error::to_string(errcode).c_str());
}

bool CDbProcess::connect_db()
{
    try
    {
        _mysql.set_host(_dbinfo.host, static_cast<uint16_t>(_dbinfo.port));
        _mysql.set_user(_dbinfo.user, _dbinfo.password);
        _mysql.set_db_name(_dbinfo.name);
        _mysql.set_charset(_dbinfo.charset);
        _mysql.enable_auto_reconnect();
        _mysql.open();

        _db_connected = true;
        _consecutive_failures = 0;
        MYLOG_INFO("dbprocess(%u, %s) connect %s ok\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), _mysql.str().c_str());
        return true;
    }
    catch (sys::CDBException& ex)
    {
        _db_connected = false;
        if (0 == _consecutive_failures++%60)
        {
            MYLOG_ERROR("dbprocess(%u, %s) connect %s error(%u): %s\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), _mysql.str().c_str(), _consecutive_failures, ex.str().c_str());
        }

        return false;
    }
}

}} // namespace mooon { namespace db_proxy {
