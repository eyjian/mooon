// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_HANDLER_H
#define MOOON_DB_PROXY_HANDLER_H
#include "rpc/DbProxyService.h" // 执行cmake或make rpc时生成的文件
namespace mooon { namespace db_proxy {

class CDbProxyHandler: public DbProxyServiceIf
{
private:
    virtual void query(DBTable& _return, const std::string& sign, const int32_t seq, const int32_t query_index, const std::vector<std::string> & tokens, const int32_t limit, const int32_t limit_start);
    virtual int update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string> & tokens);
};

} // namespace mooon
} // namespace db_proxy
#endif // MOOON_DB_PROXY_HANDLER_H
