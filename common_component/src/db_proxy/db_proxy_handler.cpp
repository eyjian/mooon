// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "db_proxy_handler.h"
#include "config_loader.h"
#include <mooon/sys/log.h>
#include <mooon/sys/simple_db.h>
#include <mooon/utils/format_string.h>
#include <mooon/utils/string_utils.h>
#include <thrift/TApplicationException.h>
#include <vector>
namespace mooon { namespace db_proxy {

void CDbProxyHandler::query(DBTable& _return, const std::string& sign, const int32_t seq, const int32_t query_index, const std::vector<std::string> & tokens, const int32_t limit, const int32_t limit_start)
{
    CConfigLoader* config_loader = CConfigLoader::get_singleton();
    struct QueryInfo query_info;
    std::vector<std::string> escaped_tokens;

    if (!config_loader->get_query_info(query_index, &query_info))
    {
        MYLOG_ERROR("query_index[%d] not exists\n", query_index);
        throw apache::thrift::TApplicationException("query_index not exists");
    }
    if (sign != query_info.sign)
    {
        MYLOG_ERROR("sign[%s] error: %s\n", sign.c_str(), query_info.sign.c_str());
        throw apache::thrift::TApplicationException("sign error");
    }

    try
    {
        sys::DBConnection* db_connection = config_loader->get_db_connection(query_info.database_index);
        if (NULL == db_connection)
        {
            MYLOG_ERROR("database_index[%d] not exists\n", query_info.database_index);
            throw apache::thrift::TApplicationException("database_index not exists");
        }
        else if (tokens.size() > utils::FORMAT_STRING_SIZE)
        {
            MYLOG_ERROR("[%d]too big: %d\n", seq, (int)tokens.size());
            throw apache::thrift::TApplicationException("tokens too many");
        }
        else
        {
            std::vector<std::string> escaped_tokens;
            escape_tokens(db_connection, tokens, &escaped_tokens);
            std::string sql = utils::format_string(query_info.sql_template.c_str(), escaped_tokens);

            MYLOG_DEBUG("%s LIMIT %d,%d\n", sql.c_str(), limit_start, limit);
            if (limit_start > 0)
                db_connection->query(_return, "%s LIMIT %d,%d", sql.c_str(), limit_start, limit);
            else
                db_connection->query(_return, "%s LIMIT %d", sql.c_str(), limit);
        }
    }
    catch (sys::CDBException& db_ex)
    {
        MYLOG_ERROR("[%d]%s\n", seq, db_ex.str().c_str());
        throw apache::thrift::TApplicationException(db_ex.str());
    }
}

int CDbProxyHandler::update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string>& tokens)
{
    return do_update(true, sign, seq, update_index, tokens);
}

void CDbProxyHandler::async_update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string>& tokens)
{
    // 异步版本，忽略返回值，
    // 降低了可靠性，提升了性能。
    (void)do_update(false, sign, seq, update_index, tokens);
}

void CDbProxyHandler::escape_tokens(void* db_connection, const std::vector<std::string>& tokens, std::vector<std::string>* escaped_tokens)
{
    sys::DBConnection* db_connection_ = (sys::DBConnection*)db_connection;

    escaped_tokens->resize(tokens.size());
    for (std::vector<std::string>::size_type i=0; i<tokens.size(); ++i)
    {
        const std::string& token = tokens[i];
        std::string escape_token = db_connection_->escape_string(token);
        (*escaped_tokens)[i] = escape_token;
    }
}

// 对于异步调用，不抛异常，因为异步调用不会等待该函数执行，也就是收不到抛出的异常
int CDbProxyHandler::do_update(bool throw_exception, const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string>& tokens)
{
    CConfigLoader* config_loader = CConfigLoader::get_singleton();
    struct UpdateInfo update_info;

    if (!config_loader->get_update_info(update_index, &update_info))
    {
        MYLOG_ERROR("[%d]update_index[%d] not exists\n", seq, update_index);
        if (throw_exception)
            throw apache::thrift::TApplicationException("update_index not exists");
    }
    else
    {
        try
        {
            sys::DBConnection* db_connection = config_loader->get_db_connection(update_info.database_index);
            if (NULL == db_connection)
            {
                MYLOG_ERROR("[%d]database_index[%d] not exists\n", seq, update_info.database_index);
                if (throw_exception)
                    throw apache::thrift::TApplicationException("database_index not exists");
            }
            else if (tokens.size() > utils::FORMAT_STRING_SIZE)
            {
                MYLOG_ERROR("[%d]too big: %d\n", seq, (int)tokens.size());
                if (throw_exception)
                    throw apache::thrift::TApplicationException("tokens too many");
            }
            else
            {
                std::vector<std::string> escaped_tokens;
                escape_tokens(db_connection, tokens, &escaped_tokens);
                std::string sql = utils::format_string(update_info.sql_template.c_str(), escaped_tokens);

                MYLOG_DEBUG("%s\n", sql.c_str());
                int affected_rows = db_connection->update("%s", sql.c_str());
                return affected_rows;
            }
        }
        catch (sys::CDBException& db_ex)
        {
            MYLOG_ERROR("[%d]%s\n", seq, db_ex.str().c_str());
            if (throw_exception)
                throw apache::thrift::TApplicationException(db_ex.str());
        }
    }

    return -1;
}

} // namespace mooon
} // namespace db_proxy
