// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "db_proxy_handler.h"
#include "config_loader.h"
#include "sql_logger.h"
#include <mooon/observer/observer_manager.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/log.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/format_string.h>
#include <mooon/utils/string_utils.h>
#include <thrift/TApplicationException.h>
#include <vector>
namespace mooon { namespace db_proxy {

CDbProxyHandler::CDbProxyHandler()
{
    mooon::observer::IObserverManager* observer_mananger = mooon::observer::get();
    if (observer_mananger != NULL)
        observer_mananger->register_observee(this);

    reset();
}

CDbProxyHandler::~CDbProxyHandler()
{
    mooon::observer::IObserverManager* observer_mananger = mooon::observer::get();
    if (observer_mananger != NULL)
        observer_mananger->deregister_objservee(this);
}

void CDbProxyHandler::query(DBTable& _return, const std::string& sign, const int32_t seq, const int32_t query_index, const std::vector<std::string> & tokens, const int32_t limit, const int32_t limit_start)
{
    CConfigLoader* config_loader = CConfigLoader::get_singleton();
    struct QueryInfo query_info;
    std::vector<std::string> escaped_tokens;

    try
    {
        if (!config_loader->get_query_info(query_index, &query_info))
        {
            MYLOG_ERROR("query_index[%d] not exists\n", query_index);
            throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("query_index(%d) not exists", query_index));
        }
        if (sign != query_info.sign)
        {
            MYLOG_ERROR("sign[%s] error: %s\n", sign.c_str(), query_info.sign.c_str());
            throw apache::thrift::TApplicationException("sign error");
        }

        try
        {
            sys::CMySQLConnection* db_connection = config_loader->get_db_connection(query_info.database_index);
            if (NULL == db_connection)
            {
                MYLOG_ERROR("database_index[%d] not exists or cannot connect\n", query_info.database_index);
                throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("database_index(%d) not exists or cannot connect", query_info.database_index));
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
                const std::string sql = utils::format_string(query_info.sql_template.c_str(), escaped_tokens);

                if (sql.empty())
                {
                    MYLOG_ERROR("error number of tokens or template: %s\n", query_info.str().c_str());
                    throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("error number of tokens or invalid template(%s)", query_info.str().c_str()));
                }
                else
                {
                    MYLOG_DEBUG("%s LIMIT %d,%d\n", sql.c_str(), limit_start, limit);
                    if (limit_start >= 0) // limit_start是从0开始而不是1
                    {
                        // 限制一次性返回的记录数太多将db_proxy搞死
                        if (limit - limit_start <= MAX_LIMIT)
                        {
                            db_connection->query(_return, "%s LIMIT %d,%d", sql.c_str(), limit_start, limit);
                        }
                        else
                        {
                            MYLOG_ERROR("limit(%d) - limit_start(%d) > %d: %s\n", limit, limit_start, MAX_LIMIT, query_info.str().c_str());
                            throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("limit(%d) - limit_start(%d) > %d", limit, limit_start, MAX_LIMIT));
                        }
                    }
                    else
                    {
                        if (limit <= MAX_LIMIT)
                        {
                            db_connection->query(_return, "%s LIMIT %d", sql.c_str(), limit);
                        }
                        else
                        {
                            MYLOG_ERROR("limit(%d) > %d: %s\n", limit, MAX_LIMIT, query_info.str().c_str());
                            throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("limit(%d) > %d", limit, MAX_LIMIT));
                        }
                    }

                    ++_num_query_success;
                }
            }
        }
        catch (sys::CDBException& db_ex)
        {
            MYLOG_ERROR("[%d]%s\n", seq, db_ex.str().c_str());
            throw apache::thrift::TApplicationException(db_ex.str());
        }
    }
    catch (...)
    {
        ++_num_query_failure;
        throw;
    }
}

int CDbProxyHandler::update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string>& tokens)
{
    int ret = -1;

    try
    {
        ret = do_update(true, sign, seq, update_index, tokens);
        if (-1 == ret)
            ++_num_update_failure;
        else
            ++_num_update_success;
    }
    catch (...)
    {
        ++_num_update_failure;
        throw;
    }

    return ret;
}

void CDbProxyHandler::async_update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string>& tokens)
{
    // 异步版本，忽略返回值，
    // 降低了可靠性，提升了性能。
    int ret = do_update(false, sign, seq, update_index, tokens);
    if (-1 == ret)
        ++_num_async_update_failure;
    else
        ++_num_async_update_success;
}

void CDbProxyHandler::escape_tokens(void* db_connection, const std::vector<std::string>& tokens, std::vector<std::string>* escaped_tokens)
{
    sys::DBConnection* db_connection_ = (sys::DBConnection*)db_connection;

    escaped_tokens->clear();
    escaped_tokens->resize(tokens.size());
    for (std::vector<std::string>::size_type i=0; i<tokens.size(); ++i)
    {
        std::string escape_token;
        const std::string& token = tokens[i];
        if (NULL == db_connection_)
            sys::CMySQLConnection::escape_string(token, &escape_token);
        else
            escape_token = db_connection_->escape_string(token);
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
            throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("update_index(%d) not exists", update_index));
    }
    else
    {
        std::vector<std::string> escaped_tokens;
        struct DbInfo db_info;
        config_loader->get_db_info(update_info.database_index, &db_info);

        if (!db_info.alias.empty())
        {
            CSqlLogger* sql_logger = config_loader->get_sql_logger(update_info.database_index);
            if (NULL == sql_logger)
            {
                if (throw_exception)
                    throw apache::thrift::TApplicationException("no sql logger");
                return 0;
            }
            else
            {
                // 写入文件由dbprocess写入db
                escape_tokens(NULL, tokens, &escaped_tokens);
                std::string sql = utils::format_string(update_info.sql_template.c_str(), escaped_tokens);
                MYLOG_DEBUG("%s\n", sql.c_str());
                //sql.append(";\n");

                bool written = sql_logger->write_log(sql);
                config_loader->release_sql_logger(sql_logger);
                if (!written && throw_exception)
                    throw apache::thrift::TApplicationException("io error");
                return 0;
            }
        }
        else
        {
            // 直接入库
            const int max_retries = 3;

            for (int retries=0; retries<max_retries; ++retries)
            {
                sys::DBConnection* db_connection = config_loader->get_db_connection(update_info.database_index);

                try
                {
                    if (NULL == db_connection)
                    {
                        MYLOG_ERROR("[%d]database_index[%d] not exists or cannot connect\n", seq, update_info.database_index);
                        if (throw_exception)
                            throw apache::thrift::TApplicationException(utils::CStringUtils::format_string("database_index(%d) not exists or cannot connect", update_info.database_index));
                        break; // 连接未成功不重试，原因是get_db_connection已做了重试连接
                    }
                    else if (tokens.size() > utils::FORMAT_STRING_SIZE)
                    {
                        MYLOG_ERROR("[%d]too big: %d\n", seq, (int)tokens.size());
                        if (throw_exception)
                            throw apache::thrift::TApplicationException("tokens too many");
                    }
                    else
                    {
                        escape_tokens(db_connection, tokens, &escaped_tokens);
                        const std::string sql = utils::format_string(update_info.sql_template.c_str(), escaped_tokens);

                        MYLOG_DEBUG("%s\n", sql.c_str());
                        int affected_rows = db_connection->update("%s", sql.c_str());
                        return affected_rows;
                    }
                }
                catch (sys::CDBException& db_ex)
                {
                    if (!db_connection->is_disconnected_exception(db_ex) || (retries==max_retries-1))
                    {
                        MYLOG_ERROR("[%d]%s\n", seq, db_ex.str().c_str());
                        if (throw_exception)
                            throw apache::thrift::TApplicationException(db_ex.str());
                        break;
                    }
                    else
                    {
                        MYLOG_ERROR("[retry][%d]%s\n", seq, db_ex.str().c_str());
                        config_loader->release_db_connection(update_info.database_index);
                        mooon::sys::CUtils::millisleep(100); // 网络类原因稍后重试
                    }
                }
            } // for
        }
    }

    return -1;
}

void CDbProxyHandler::on_report(mooon::observer::IDataReporter* data_reporter)
{
    if ((_num_query_success != 0) || (_num_query_failure != 0) ||
        (_num_update_success != 0) || (_num_update_failure != 0) ||
        (_num_async_update_success != 0) || (_num_async_update_failure != 0))
    {
        data_reporter->report("[%s]%d,%d,%d,%d,%d,%d\n", sys::get_formatted_current_datetime(true).c_str(),
            _num_query_success, _num_query_failure,
            _num_update_success, _num_update_failure,
            _num_async_update_success, _num_async_update_failure);
        reset();
    }
}

void CDbProxyHandler::reset()
{
    _num_query_success = 0;
    _num_query_failure = 0;
    _num_update_success = 0;
    _num_update_failure = 0;
    _num_async_update_success = 0;
    _num_async_update_failure = 0;
}

} // namespace mooon
} // namespace db_proxy
