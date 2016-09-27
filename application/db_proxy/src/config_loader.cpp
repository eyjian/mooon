// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include "sql_logger.h"
#include <errno.h>
#include <fstream>
#include <mooon/net/utils.h>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/md5_helper.h>
#include <mooon/utils/string_utils.h>
#include <mooon/utils/tokener.h>
#include <set>
#include <sys/inotify.h> // 一些低版本内核没有实现
#include <vector>
namespace mooon { namespace db_proxy {

bool is_sql_log_filename(const std::string& filename)
{
    std::vector<std::string> tokens;
    utils::CTokener::split(&tokens, filename, ".");
    return (3 == tokens.size()) && (tokens[0] == "sql") &&
           (12 == tokens[1].size()) &&
           (6 == tokens[2].size()) &&
           utils::CStringUtils::is_numeric_string(tokens[1].c_str()) &&
           utils::CStringUtils::is_numeric_string(tokens[2].c_str());
}

std::string get_log_dirpath(const std::string& alias)
{
    const std::string program_path = sys::CUtils::get_program_path();
    return utils::CStringUtils::format_string("%s/../%s/%s", program_path.c_str(), SQLLOG_DIRNAME, alias.c_str());
}

// 线程级DB连接
static __thread sys::CMySQLConnection* g_db_connection[MAX_DB_CONNECTION] = { NULL } ;

static void init_db_info_array(struct DbInfo* db_info_array[])
{
    for (int index=0; index<MAX_DB_CONNECTION; ++index)
        db_info_array[index] = NULL;
}

static void init_query_info_array(struct QueryInfo* query_info_array[])
{
    for (int index=0; index<MAX_SQL_TEMPLATE; ++index)
        query_info_array[index] = NULL;
}

static void init_update_info_array(struct UpdateInfo* update_info_array[])
{
    for (int index=0; index<MAX_SQL_TEMPLATE; ++index)
        update_info_array[index] = NULL;
}

static void release_db_info_array(struct DbInfo* db_info_array[])
{
    for (int index=0; index<MAX_DB_CONNECTION; ++index)
    {
        delete db_info_array[index];
        db_info_array[index] = NULL;
    }
}

static void release_sql_logger_array(CSqlLogger* sql_logger_array[])
{
    for (int index=0; index<MAX_DB_CONNECTION; ++index)
    {
        if (sql_logger_array[index] != NULL)
        {
            sql_logger_array[index]->dec_refcount();
            sql_logger_array[index] = NULL;
        }
    }
}

static void release_query_info_array(struct QueryInfo* query_info_array[])
{
    for (int index=0; index<MAX_SQL_TEMPLATE; ++index)
    {
        delete query_info_array[index];
        query_info_array[index] = NULL;
    }
}

static void release_update_info_array(struct UpdateInfo* update_info_array[])
{
    for (int index=0; index<MAX_SQL_TEMPLATE; ++index)
    {
        delete update_info_array[index];
        update_info_array[index] = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
SINGLETON_IMPLEMENT(CConfigLoader);

std::string CConfigLoader::get_filepath()
{
    std::string program_path = sys::CUtils::get_program_path();
    std::string filepath = program_path + "/../conf/sql.json";

    if (access(filepath.c_str(), F_OK) != 0)
    {
        MYLOG_DETAIL("%s not exist\n", filepath.c_str());
        filepath = program_path + "/sql.json";
    }

    return filepath;
}

void CConfigLoader::monitor()
{
    while (!_stop_monitor)
    {
        sys::CUtils::millisleep(2000);

        std::string filepath = CConfigLoader::get_filepath();
        (void)load(filepath);
    }
}

void CConfigLoader::stop_monitor()
{
    _stop_monitor = true;
}

CConfigLoader::CConfigLoader()
    : _stop_monitor(false)
{
    init_db_info_array(_db_info_array);
    init_query_info_array(_query_info_array);
    init_update_info_array(_update_info_array);

    // 无效md5值
    _md5_sum = "-";
}

bool CConfigLoader::load(const std::string& filepath)
{
    Json::Reader reader;
    Json::Value root;
    std::ifstream fs(filepath.c_str());
    struct DbInfo* db_info_array[MAX_DB_CONNECTION] = { NULL };
    struct QueryInfo* query_info_array[MAX_SQL_TEMPLATE] = { NULL };
    struct UpdateInfo* update_info_array[MAX_SQL_TEMPLATE] = { NULL };

    if (_md5_sum.empty())
    {
        MYLOG_INFO("loading %s\n", filepath.c_str());
    }
    else
    {
        MYLOG_DETAIL("loading %s\n", filepath.c_str());
    }
    if (!fs)
    {
        MYLOG_ERROR("load %s failed: %s\n", filepath.c_str(), strerror(errno));
        return false;
    }
    if (!reader.parse(fs, root))
    {
        MYLOG_ERROR("parse %s failed: %s\n", filepath.c_str(), reader.getFormattedErrorMessages().c_str());
        return false;
    }

    // 检查文件是否有修改过
    std::string md5_sum = utils::CMd5Helper::lowercase_md5("%s", root.toStyledString().c_str());
    if (md5_sum == _md5_sum)
    {
        MYLOG_DETAIL("not changed: (%s)%s\n", md5_sum.c_str(), filepath.c_str());
        return true; // 未发生变化
    }

    init_db_info_array(db_info_array);
    init_query_info_array(query_info_array);
    init_update_info_array(update_info_array);
    if (!load_database(root["database"], db_info_array))
        return false;
    if (!load_query(root["query"], query_info_array))
        return false;
    if (!load_update(root["update"], update_info_array))
        return false;

    // 加写锁
    sys::WriteLockHelper write_lock(_read_write_lock);
    release_db_info_array(_db_info_array);
    release_sql_logger_array(_sql_logger_array);
    release_query_info_array(_query_info_array);
    release_update_info_array(_update_info_array);

    for (int index=0; index<MAX_DB_CONNECTION; ++index)
    {
        if (db_info_array[index] != NULL)
        {
            // 启动时即连接一下，以早期发现配置等问题
            _db_info_array[index] = new struct DbInfo(*db_info_array[index]);
            sys::DBConnection* db_connection = init_db_connection(index, false);
            if (db_connection != NULL)
            {
                delete db_connection;
                db_connection = NULL;
            }

            // 创建好SqlLogger
            CSqlLogger* sql_logger = new CSqlLogger(index, _db_info_array[index]);
            _sql_logger_array[index] = sql_logger;
            sql_logger->inc_refcount();
        }
    }
    for (int index=0; index<MAX_SQL_TEMPLATE; ++index)
    {
        if (query_info_array[index] != NULL)
            _query_info_array[index] = new struct QueryInfo(*query_info_array[index]);
        if (update_info_array[index] != NULL)
            _update_info_array[index] = new struct UpdateInfo(*update_info_array[index]);
    }

    _md5_sum = md5_sum;
    MYLOG_INFO("loaded %s[%s] successfully\n", filepath.c_str(), _md5_sum.c_str());
    return true;
}

CSqlLogger* CConfigLoader::get_sql_logger(int index)
{
    CSqlLogger* sql_logger = NULL;

    if ((index >= 0) && (index < MAX_DB_CONNECTION))
    {
        {
            sys::ReadLockHelper read_lock(_read_write_lock);
            sql_logger = _sql_logger_array[index];
            if (sql_logger != NULL)
                sql_logger->inc_refcount();
        }

        if (NULL == sql_logger)
        {
            sys::WriteLockHelper write_lock(_read_write_lock);
            sql_logger = _sql_logger_array[index];
            if (sql_logger != NULL)
            {
                sql_logger->inc_refcount();
            }
            else
            {
                struct DbInfo* dbinfo = _db_info_array[index];
                if (dbinfo != NULL)
                {
                    sql_logger = new CSqlLogger(index, dbinfo);
                    _sql_logger_array[index] = sql_logger;
                    sql_logger->inc_refcount();
                }
            }
        }
    }

    return sql_logger;
}

void CConfigLoader::release_sql_logger(CSqlLogger* sql_logger)
{
    if (sql_logger != NULL)
    {
        const std::string str = sql_logger->str();
        const int index = sql_logger->get_database_index();

        sys::ReadLockHelper read_lock(_read_write_lock);
        if (sql_logger->dec_refcount())
        {
            MYLOG_WARN("deleted sqllogger: %s\n", str.c_str());
            if (sql_logger == _sql_logger_array[index])
                _sql_logger_array[index] = NULL;
        }
    }
}

void CConfigLoader::release_db_connection(int index)
{
    if ((index >= 0) && (index < MAX_DB_CONNECTION))
    {
        delete g_db_connection[index];
        g_db_connection[index] = NULL;
    }
}

sys::CMySQLConnection* CConfigLoader::get_db_connection(int index) const
{
    if ((index < 0) || (index >= MAX_DB_CONNECTION))
    {
        MYLOG_ERROR("invalid database index: %d\n", index);
        return NULL;
    }
    if (NULL == g_db_connection[index])
    {
        g_db_connection[index] = init_db_connection(index, true);
    }

    return g_db_connection[index];
}

bool CConfigLoader::get_db_info(int index, struct DbInfo* db_info) const
{
    sys::ReadLockHelper read_lock(_read_write_lock);

    if ((index < 0) || (index >= MAX_DB_CONNECTION))
        return false;
    if (NULL == _db_info_array[index])
        return false;

    *db_info = *(_db_info_array[index]);
    return true;
}

bool CConfigLoader::get_query_info(int index, struct QueryInfo* query_info) const
{
    sys::ReadLockHelper read_lock(_read_write_lock);
    if ((index < 0) || (index >= MAX_SQL_TEMPLATE))
        return false;
    if (NULL == _query_info_array[index])
        return false;

    *query_info = *(_query_info_array[index]);
    return true;
}

bool CConfigLoader::get_update_info(int index, struct UpdateInfo* update_info) const
{
    sys::ReadLockHelper read_lock(_read_write_lock);
    if ((index < 0) || (index >= MAX_SQL_TEMPLATE))
        return false;
    if (NULL == _update_info_array[index])
        return false;

    *update_info = *(_update_info_array[index]);
    return true;
}

bool CConfigLoader::load_database(const Json::Value& json, struct DbInfo* db_info_array[])
{
    std::set<std::string> alias_set;

    for (int i=0; i<static_cast<int>(json.size()); ++i)
    {
        struct DbInfo* db_info = new struct DbInfo(json[i]);

        if (!db_info->check())
        {
            delete db_info;
            return false;
        }
        else
        {
            MYLOG_INFO("%s\n", db_info->str().c_str());

            if (!db_info->alias.empty())
            {
                std::pair<std::set<std::string>::iterator, bool> ret = alias_set.insert(db_info->alias);
                if (!ret.second)
                {
                    // 同名的别名已存在
                    MYLOG_ERROR("alias exists: %s\n", db_info->alias.c_str());
                    db_info->alias = std::string(INVALID_ALIAS_PREFIX) + db_info->alias; // 设置为无效别名
                }
            }
            if (!add_db_info(db_info, db_info_array))
            {
                delete db_info;
                return false;
            }
        }
    }

    return true;
}

bool CConfigLoader::load_query(const Json::Value& json, struct QueryInfo* query_info_array[])
{
    for (int i=0; i<static_cast<int>(json.size()); ++i)
    {
        struct QueryInfo* query_info = new struct QueryInfo(json[i]);

        if (!query_info->check())
        {
            delete query_info;
            return false;
        }
        else
        {
            MYLOG_INFO("%s\n", query_info->str().c_str());
            if (!add_query_info(query_info, query_info_array))
            {
                delete query_info;
                return false;
            }
        }
    }

    return true;
}

bool CConfigLoader::load_update(const Json::Value& json, struct UpdateInfo* update_info_array[])
{
    for (int i=0; i<static_cast<int>(json.size()); ++i)
    {
        struct UpdateInfo* update_info = new struct UpdateInfo(json[i]);

        if (!update_info->check())
        {
            delete update_info;
            return false;
        }
        else
        {
            MYLOG_INFO("%s\n", update_info->str().c_str());
            if (!add_update_info(update_info, update_info_array))
            {
                delete update_info;
                return false;
            }
        }
    }

    return true;
}

bool CConfigLoader::add_db_info(struct DbInfo* db_info, struct DbInfo* db_info_array[])
{
    int index = db_info->index;

    if (index >= MAX_DB_CONNECTION)
    {
        MYLOG_ERROR("index[%d] greater or equal %d of %s\n", index, MAX_DB_CONNECTION, db_info->str().c_str());
        return false;
    }
    if (db_info_array[index] != NULL)
    {
        MYLOG_ERROR("index[%d] repeat: %s => %s\n", index, db_info->str().c_str(), db_info_array[index]->str().c_str());
        return false;
    }

    db_info_array[index] = db_info;
    return true;
}

bool CConfigLoader::add_query_info(struct QueryInfo* query_info, struct QueryInfo* query_info_array[])
{
    int index = query_info->index;

    if (index >= MAX_SQL_TEMPLATE)
    {
        MYLOG_ERROR("index[%d] greater or equal %d of %s\n", index, MAX_SQL_TEMPLATE, query_info->str().c_str());
        return false;
    }
    if (query_info_array[index] != NULL)
    {
        MYLOG_ERROR("index[%d] repeat: %s => %s\n", index, query_info->str().c_str(), query_info_array[index]->str().c_str());
        return false;
    }

    query_info_array[index] = query_info;
    return true;
}

bool CConfigLoader::add_update_info(struct UpdateInfo* update_info, struct UpdateInfo* update_info_array[])
{
    int index = update_info->index;

    if (index >= MAX_SQL_TEMPLATE)
    {
        MYLOG_ERROR("index[%d] greater or equal %d of %s\n", index, MAX_SQL_TEMPLATE, update_info->str().c_str());
        return false;
    }
    if (update_info_array[index] != NULL)
    {
        MYLOG_ERROR("index[%d] repeat: %s => %s\n", index, update_info->str().c_str(), update_info_array[index]->str().c_str());
        return false;
    }

    update_info_array[index] = update_info;
    return true;
}

sys::CMySQLConnection* CConfigLoader::init_db_connection(int index, bool need_lock) const
{
    if (!need_lock)
    {
        return do_init_db_connection(index);
    }
    else
    {
        sys::ReadLockHelper read_lock(_read_write_lock);
        return do_init_db_connection(index);
    }
}

sys::CMySQLConnection* CConfigLoader::do_init_db_connection(int index) const
{
    const int max_retries = 3;
    const struct DbInfo* _db_info = _db_info_array[index];
    sys::CMySQLConnection* db_connection = NULL;

    for (int retries=0; retries<max_retries; ++retries)
    {
        db_connection = new sys::CMySQLConnection;
        db_connection->set_host(_db_info->host, (uint16_t)_db_info->port);
        db_connection->set_user(_db_info->user, _db_info->password);
        db_connection->set_db_name(_db_info->name);
        db_connection->set_charset(_db_info->charset);
        db_connection->enable_auto_reconnect();

        try
        {
            db_connection->open();
            break;
        }
        catch (sys::CDBException& db_ex)
        {
            bool is_disconnected_exception = db_connection->is_disconnected_exception(db_ex);
            delete db_connection;
            db_connection = NULL;

            if (!is_disconnected_exception || retries==max_retries-1)
            {
                MYLOG_ERROR("connect %s failed: %s\n", _db_info->str().c_str(), db_ex.str().c_str());
                break;
            }
            else
            {
                MYLOG_ERROR("connect %s failed to retry: %s\n", _db_info->str().c_str(), db_ex.str().c_str());
                mooon::sys::CUtils::millisleep(100); // 网络类原因稍后重试
            }
        }
    }

    return db_connection;
}

} // namespace db_proxy
} // namespace mooon
