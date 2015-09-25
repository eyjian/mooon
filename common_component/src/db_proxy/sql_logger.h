// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_SQL_LOGGER_H
#define MOOON_DB_PROXY_SQL_LOGGER_H
#include <mooon/sys/lock.h>
namespace mooon { namespace db_proxy {

struct DbInfo;

// 将SQL写入到SQL专用日志文件中
// SQL日志存放目录为：
// homedir/sql_log/db_host_db_name/current/db_host_db_name_YYYYMMDDHHmmss.sql
// homedir为程序文件所在目录的父目录，运行时会自动创建所需要的目录，但运行前就需要保证能够创建成功
// 与CSqlLogger协同者会将这些SQL写入DB，并在完成后将SQL日志文件移到目录：
// homedir/sql_log/db_host_db_name/year，即按年归档存放
class CSqlLogger
{
public:
    SINGLETON_DECLARE(CSqlLogger);
    static uint32_t sql_log_filesize; // 每个SQL日志文件的最大大小，单位为字节数

public:
    void write_log(int database_index, const std::string& sql);

private:
    int get_fd(int database_index);
    int open_sql_log(const std::string& current_filepath);
    int open_sql_log(int database_index);
    std::string get_current_filepath(const struct DbInfo& db_info) const;
    std::string get_rolled_filepath(const struct DbInfo& db_info, const std::string& datetime) const;
    bool need_roll(int fd) const; // 是否需要滚动文件了
    void roll(int database_index);

private:
    sys::CLock _lock;
    std::string _log_filepath;
};

} // namespace db_proxy
} // namespace mooon
#endif // MOOON_DB_PROXY_SQL_LOGGER_H
