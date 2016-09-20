// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_DB_PROCESS_H
#define MOOON_DB_PROXY_DB_PROCESS_H
#include "config_loader.h"
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/thread_engine.h>
namespace mooon { namespace db_proxy {

// 父进程不在时自动退出
class CDbProcess
{
public:
    CDbProcess(const struct DbInfo& dbinfo);
    ~CDbProcess();
    void run();

private:
    void signal_thread();

public:
    void on_terminated();
    void on_child_end(pid_t child_pid, int child_exited_status);
    void on_signal_handler(int signo);
    void on_exception(int errcode) throw ();

private:
    void handle_directory();
    void handle_file(const std::string& filename);
    bool connect_db();

private:
    struct DbInfo _dbinfo;
    std::string _log_dirpath; // 日志存放目录
    volatile bool _stop_signal_thread;
    sys::CThreadEngine* _signal_thread;
    sys::CMySQLConnection _mysql;
    uint32_t _consecutive_failures; // 用于减少连接DB失败时的重复日志
    bool _db_connected; // 是否连接了DB
};

}} // namespace mooon { namespace db_proxy {
#endif // MOOON_DB_PROXY_DB_PROCESS_H
