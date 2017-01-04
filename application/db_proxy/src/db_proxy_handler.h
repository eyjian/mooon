// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_HANDLER_H
#define MOOON_DB_PROXY_HANDLER_H
#include "DbProxyService.h" // 执行cmake或make rpc时生成的文件
#include <mooon/observer/observable.h>
#include <mooon/sys/atomic.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/read_write_lock.h>
#include <mooon/utils/md5_helper.h>
#include <tr1/unordered_map>
namespace mooon { namespace db_proxy {

struct CachedData
{
    time_t timestamp; // 时间戳
    int cached_seconds; // 缓存时长
    sys::DBTable cached_data; // 被缓存的数据
};

class CDbProxyHandler: public DbProxyServiceIf, public mooon::observer::IObservable
{
public:
    CDbProxyHandler();
    ~CDbProxyHandler();

    // 清理缓存
    void cleanup_cache();

private: // override DbProxyServiceIf
    virtual void query(DBTable& _return, const std::string& sign, const int32_t seq, const int32_t query_index, const std::vector<std::string> & tokens, const int32_t limit, const int32_t limit_start);
    virtual int update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string> & tokens);
    virtual void async_update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string> & tokens);

private: // override mooon::observer::IObservable
    virtual void on_report(mooon::observer::IDataReporter* data_reporter);

private:
    // 对字符串进行编码，以防止SQL注入
    // 参数db_connection不能为NULL
    void escape_tokens(void* db_connection, const std::vector<std::string>& tokens, std::vector<std::string>* escaped_tokens);

    // throw_exception 是否抛异常，对于同步版本需要抛，异步版本不抛
    int do_update(bool throw_exception, const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string> & tokens);

    // 从缓存中取数据，如果取到返回true，否则返回false
    bool get_data_from_cache(DBTable& dbtable, const std::string& sql);
    // 添加到缓存中
    void add_data_to_cache(const DBTable& dbtable, const std::string& sql, int cached_seconds);

private:
    void reset();
    volatile int _num_query_success;
    volatile int _num_query_failure;
    volatile int _num_update_success; // 成功的update次数
    volatile int _num_update_failure;
    volatile int _num_async_update_success;
    volatile int _num_async_update_failure;

private:
    typedef std::tr1::unordered_map<utils::CMd5Helper::Value, struct CachedData, utils::CMd5Helper::ValueHasher, utils::CMd5Helper::ValueComparer> CacheTable;
    CacheTable _cache_table; // 缓存表
    mutable sys::CReadWriteLock _cache_table_lock;
    atomic_t _cached_number; // 缓存的数据笔数
};

} // namespace mooon
} // namespace db_proxy
#endif // MOOON_DB_PROXY_HANDLER_H
