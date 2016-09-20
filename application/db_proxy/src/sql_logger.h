// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_SQL_LOGGER_H
#define MOOON_DB_PROXY_SQL_LOGGER_H
#include <mooon/sys/lock.h>
#include <mooon/sys/ref_countable.h>
namespace mooon { namespace db_proxy {

struct DbInfo;

// 实现关键点：
// 需要确保一个线程能够区分文件是否被其它线程滚动了，这可以通过大小是否变小了，fd值是否变化了来判断
class CSqlLogger: public sys::CRefCountable
{
public:
    CSqlLogger(int database_index, const struct DbInfo* dbinfo);
    ~CSqlLogger();
    std::string str() const;
    int get_database_index() const { return _database_index; }
    bool write_log(const std::string& sql);

private:
    void rotate_log();
    std::string get_log_filepath();

private:
    volatile time_t _log_file_timestamp; // 创建日志文件的时间
    volatile int _log_file_suffix; // 为防止同一秒内创建的文件超出1个，设一suffix
    int _database_index;
    struct DbInfo* _dbinfo;
    sys::CLock _lock;
    atomic_t _log_fd;
    atomic_t _total_bytes_written; // 日志文件大小
};

} // namespace db_proxy
} // namespace mooon
#endif // MOOON_DB_PROXY_SQL_LOGGER_H
