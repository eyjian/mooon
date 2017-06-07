// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_DB_PROCESS_H
#define MOOON_DB_PROXY_DB_PROCESS_H
#include "config_loader.h"
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/thread_engine.h>
#include <zlib.h>
namespace mooon { namespace db_proxy {

struct Progress
{
    uint32_t crc32;
    uint32_t offset;
    char filename[sizeof("sql.0123456789AB.123456")]; // 包含结尾符'\0'

    Progress()
    {
        crc32 = 0;
        offset = 0;
        memset(filename, sizeof(filename), 0);
    }

    bool empty() const
    {
        return '\0' == filename[0];
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string("progress://C%u/O%u/F%s", crc32, offset, filename);
    }

    uint32_t get_crc32() const
    {
        const std::string crc32_str = utils::CStringUtils::format_string("%u%s", offset, filename);
        return ::crc32(0L, (const unsigned char*)crc32_str.data(), crc32_str.size());
    }
};

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
    bool parent_process_not_exists() const;
    bool create_history_directory() const;
    void handle_directory();
    bool handle_file(const std::string& filename);
    bool file_handled(const std::string& filename) const; // 是否已处理过
    bool is_current_file(const std::string& filename) const;
    bool connect_db();
    void close_db();
    bool update_progress(const std::string& filename, uint32_t offset);
    bool get_progress(struct Progress* progress);
    bool open_progress();
    void archive_file(const std::string& filename) const;

private:
    std::string get_filepath(const std::string& filename) const;
    std::string get_archived_filepath(const std::string& filename) const;
    std::string get_history_dirpath() const;

private:
    int _progess_fd; // 进度文件句柄
    struct Progress _progress;
    struct DbInfo _dbinfo;
    std::string _log_dirpath; // 日志存放目录
    volatile bool _stop_signal_thread;
    sys::CThreadEngine* _signal_thread;
    sys::CMySQLConnection _mysql;
    uint32_t _consecutive_failures; // 用于减少连接DB失败时的重复日志
    uint64_t _num_sqls; // 启动以来总共处理过的SQL条数
    bool _db_connected; // 是否连接了DB

private:
    time_t _begin_time; // 效率统计开始时间
    int _interval_count; // 效率统计时间段内发生的数目
    int _batch; // 当前事务已写的条数
};

}} // namespace mooon { namespace db_proxy {
#endif // MOOON_DB_PROXY_DB_PROCESS_H
