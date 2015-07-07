// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include <errno.h>
#include <fstream>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/log.h>
#include <mooon/sys/utils.h>
namespace mooon { namespace db_proxy {

// 线程级DB连接
static __thread sys::DBConnection* g_db_connection[MAX_DB_CONNECTION] = { NULL } ;

SINGLETON_IMPLEMENT(CConfigLoader);

std::string CConfigLoader::get_filepath()
{
    std::string program_path = mooon::sys::CUtils::get_program_path();
    std::string filepath = program_path + "/../conf/sql.json";

    if (access(filepath.c_str(), F_OK) != 0)
    {
        MYLOG_WARN("%s not exist\n", filepath.c_str());
        filepath = program_path + "/sql.json";
    }

    return filepath;
}

CConfigLoader::CConfigLoader()
{
    init_db_info_array();
    init_query_info_array();
    init_update_info_array();
}

bool CConfigLoader::load(const std::string& filepath)
{
    Json::Reader reader;
    Json::Value root;
    std::ifstream fs(filepath.c_str());

    MYLOG_INFO("loading %s\n", filepath.c_str());
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

    if (!load_database(root["database"]))
        return false;
    if (!load_query(root["query"]))
        return false;
    if (!load_update(root["update"]))
        return false;

    MYLOG_INFO("loaded %s successfully\n", filepath.c_str());
    return true;
}

sys::DBConnection* CConfigLoader::get_db_connection(int index) const
{
    if ((index < 0) || (index >= MAX_DB_CONNECTION))
        return NULL;

    if (NULL == g_db_connection[index])
        g_db_connection[index] = init_db_connection(index);

    return g_db_connection[index];
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

void CConfigLoader::init_db_info_array()
{
    for (int i=0; i<MAX_DB_CONNECTION; ++i)
        _db_info_array[i] = NULL;
}

void CConfigLoader::init_query_info_array()
{
    for (int i=0; i<MAX_SQL_TEMPLATE; ++i)
        _query_info_array[i] = NULL;
}

void CConfigLoader::init_update_info_array()
{
    for (int i=0; i<MAX_SQL_TEMPLATE; ++i)
        _update_info_array[i] = NULL;
}

bool CConfigLoader::add_db_info(struct DbInfo* db_info)
{
    int index = db_info->index;

    if (index >= MAX_DB_CONNECTION)
    {
        MYLOG_ERROR("index[%d] greater or equal %d of %s\n", index, MAX_DB_CONNECTION, db_info->str().c_str());
        return false;
    }
    if (_db_info_array[index] != NULL)
    {
        MYLOG_ERROR("index[%d] repeat: %s => %s\n", index, db_info->str().c_str(), _db_info_array[index]->str().c_str());
        return false;
    }

    _db_info_array[index] = db_info;
    return true;
}

bool CConfigLoader::add_query_info(struct QueryInfo* query_info)
{
    int index = query_info->index;

    if (index >= MAX_SQL_TEMPLATE)
    {
        MYLOG_ERROR("index[%d] greater or equal %d of %s\n", index, MAX_SQL_TEMPLATE, query_info->str().c_str());
        return false;
    }
    if (_query_info_array[index] != NULL)
    {
        MYLOG_ERROR("index[%d] repeat: %s => %s\n", index, query_info->str().c_str(), _query_info_array[index]->str().c_str());
        return false;
    }

    _query_info_array[index] = query_info;
    return true;
}

bool CConfigLoader::add_update_info(struct UpdateInfo* update_info)
{
    int index = update_info->index;

    if (index >= MAX_SQL_TEMPLATE)
    {
        MYLOG_ERROR("index[%d] greater or equal %d of %s\n", index, MAX_SQL_TEMPLATE, update_info->str().c_str());
        return false;
    }
    if (_update_info_array[index] != NULL)
    {
        MYLOG_ERROR("index[%d] repeat: %s => %s\n", index, update_info->str().c_str(), _update_info_array[index]->str().c_str());
        return false;
    }

    _update_info_array[index] = update_info;
    return true;
}

bool CConfigLoader::load_database(const Json::Value& json)
{
    for (size_t i=0; i<json.size(); ++i)
    {
        struct DbInfo* db_info = new struct DbInfo(json[i]);

        if (!db_info->check())
        {
            MYLOG_ERROR("checked failed: %s\n", db_info->str().c_str());
            delete db_info;
            return false;
        }
        else
        {
            MYLOG_INFO("%s\n", db_info->str().c_str());
            if (!add_db_info(db_info))
            {
                delete db_info;
                return false;
            }
        }
    }

    return true;
}

bool CConfigLoader::load_query(const Json::Value& json)
{
    for (size_t i=0; i<json.size(); ++i)
    {
        struct QueryInfo* query_info = new struct QueryInfo(json[i]);

        if (!query_info->check())
        {
            MYLOG_ERROR("checked failed: %s\n", query_info->str().c_str());
            delete query_info;
            return false;
        }
        else
        {
            MYLOG_INFO("%s\n", query_info->str().c_str());
            if (!add_query_info(query_info))
            {
                delete query_info;
                return false;
            }
        }
    }

    return true;
}

bool CConfigLoader::load_update(const Json::Value& json)
{
    for (size_t i=0; i<json.size(); ++i)
    {
        struct UpdateInfo* update_info = new struct UpdateInfo(json[i]);

        if (!update_info->check())
        {
            MYLOG_ERROR("checked failed: %s\n", update_info->str().c_str());
            delete update_info;
            return false;
        }
        else
        {
            MYLOG_INFO("%s\n", update_info->str().c_str());
            if (!add_update_info(update_info))
            {
                delete update_info;
                return false;
            }
        }
    }

    return true;
}

sys::DBConnection* CConfigLoader::init_db_connection(int index) const
{
    sys::ReadLockHelper read_lock(_read_write_lock);

    const struct DbInfo* _db_info = _db_info_array[index];
    sys::DBConnection* db_connection = mooon::sys::DBConnection::create_connection("mysql");

    db_connection->set_host(_db_info->host, (uint16_t)_db_info->port);
    db_connection->set_user(_db_info->user, _db_info->password);
    db_connection->set_db_name(_db_info->name);
    db_connection->set_charset(_db_info->charset);

    db_connection->open();
    return db_connection;
}

} // namespace mooon
} // namespace db_proxy
