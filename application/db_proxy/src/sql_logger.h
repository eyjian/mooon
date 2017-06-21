// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_SQL_LOGGER_H
#define MOOON_DB_PROXY_SQL_LOGGER_H
#include <mooon/observer/observable.h>
#include <mooon/sys/lock.h>
#include <mooon/sys/ref_countable.h>
namespace mooon { namespace db_proxy {

struct DbInfo;

// 实现关键点：
// 需要确保一个线程能够区分文件是否被其它线程滚动了，这可以通过大小是否变小了，fd值是否变化了来判断
class CSqlLogger: public sys::CRefCountable, public mooon::observer::IObservable
{
public:
    CSqlLogger(int database_index, const struct DbInfo* dbinfo);
    ~CSqlLogger();

    std::string str() const;
    int get_database_index() const { return _database_index; }
    bool write_log(const std::string& sql);

private: // override mooon::observer::IObservable
    virtual void on_report(mooon::observer::IDataReporter* data_reporter, const std::string& current_datetime);

private:
    bool need_rotate() const;
    void rotate_log();
    std::string get_log_filepath();
    std::string get_last_log_filepath(); // 取得启动之前最新的一个日志文件，如果不存在则等同于get_log_filepath()

private:
    sys::CLock _lock;
    int _database_index;
    struct DbInfo* _dbinfo;
    std::string _log_filepath; // 须受_lock保护

private:
    volatile int _log_fd; // SQL日志文件句柄
    volatile time_t _log_file_timestamp; // 创建日志文件的时间
    volatile int _log_file_suffix; // 为防止同一秒内创建的文件超出1个，设一suffix
    volatile int32_t _num_lines; // 连续写入的行数
    volatile uint64_t _total_lines; // 自启动以来总的写入行数
    volatile uint64_t _last_total_lines; // 上一次report时的总数
};

} // namespace db_proxy
} // namespace mooon
#endif // MOOON_DB_PROXY_SQL_LOGGER_H
