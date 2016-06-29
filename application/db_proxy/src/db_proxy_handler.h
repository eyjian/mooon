// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_HANDLER_H
#define MOOON_DB_PROXY_HANDLER_H
#include "rpc/DbProxyService.h" // 执行cmake或make rpc时生成的文件
#include <mooon/observer/observable.h>
namespace mooon { namespace db_proxy {

class CDbProxyHandler: public DbProxyServiceIf, public mooon::observer::IObservable
{
public:
    CDbProxyHandler();
    ~CDbProxyHandler();

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

private:
    void reset();
    volatile int _num_query_success;
    volatile int _num_query_failure;
    volatile int _num_update_success; // 成功的update次数
    volatile int _num_update_failure;
    volatile int _num_async_update_success;
    volatile int _num_async_update_failure;
};

} // namespace mooon
} // namespace db_proxy
#endif // MOOON_DB_PROXY_HANDLER_H
