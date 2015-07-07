// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_CONFIG_LOADER_H
#define MOOON_DB_PROXY_CONFIG_LOADER_H
#include <json/json.h>
#include <mooon/sys/read_write_lock.h>
#include <mooon/sys/simple_db.h>
#include <mooon/utils/string_utils.h>
#include <string>
namespace mooon { namespace db_proxy {

// 定义常量
enum
{
    MAX_DB_CONNECTION = 100, // 单个线程最多支持的DB连接个数
    MAX_SQL_TEMPLATE = 10000 // 单个线程最多支持的SQL模板个数
};

struct DbInfo
{
    int index;
    std::string host;
    int port;
    std::string name;
    std::string user;
    std::string password;
    std::string charset;

    DbInfo(const Json::Value& json)
    {
        index = json["index"].asInt();
        host = json["host"].asString();
        port = json["port"].asInt();
        name = json["name"].asString();
        user = json["user"].asString();
        password = json["password"].asString();
        charset = json["charset"].asString();
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string(
            "database://%d/%s/%d/%s/%s", index, host.c_str(), port, name.c_str(), user.c_str());
    }

    bool check() const
    {
        return true;
    }
};

struct QueryInfo
{
    int index;
    int database_index;
    int cached_seconds;
    std::string sql_template;

    QueryInfo()
    {
        index = -1;
        database_index = -1;
        cached_seconds = -1;
    }

    QueryInfo(const Json::Value& json)
    {
        index = json["index"].asInt();
        database_index = json["database_index"].asInt();
        cached_seconds = json["cached_seconds"].asInt();
        sql_template = json["sql_template"].asString();
    }

    QueryInfo(const QueryInfo& other)
    {
        index = other.index;
        database_index = other.database_index;
        cached_seconds = other.cached_seconds;
        sql_template = other.sql_template;
    }

    QueryInfo& operator =(const QueryInfo& other)
    {
        index = other.index;
        database_index = other.database_index;
        cached_seconds = other.cached_seconds;
        sql_template = other.sql_template;

        return *this;
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string(
            "query://%d/%d/%d/`%s`", index, database_index, cached_seconds, sql_template.c_str());
    }

    bool check() const
    {
        return true;
    }
};

struct UpdateInfo
{
    int index;
    int database_index;
    std::string sql_template;

    UpdateInfo()
    {
        index = -1;
        database_index = -1;
    }

    UpdateInfo(const Json::Value& json)
    {
        index = json["index"].asInt();
        database_index = json["database_index"].asInt();
        sql_template = json["sql_template"].asString();
    }

    UpdateInfo(const QueryInfo& other)
    {
        index = other.index;
        database_index = other.database_index;
        sql_template = other.sql_template;
    }

    UpdateInfo& operator =(const UpdateInfo& other)
    {
        index = other.index;
        database_index = other.database_index;
        sql_template = other.sql_template;

        return *this;
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string(
            "update://%d/%d/`%s`", index, database_index, sql_template.c_str());
    }

    bool check() const
    {
        return true;
    }
};

// 负责配置的加载
class CConfigLoader
{
public:
    SINGLETON_DECLARE(CConfigLoader);
    static std::string get_filepath();

public:
    CConfigLoader();
    bool load(const std::string& filepath);
    sys::DBConnection* get_db_connection(int index) const;
    bool get_query_info(int index, struct QueryInfo* query_info) const;
    bool get_update_info(int index, struct UpdateInfo* update_info) const;

private:
    void init_db_info_array();
    void init_query_info_array();
    void init_update_info_array();
    bool add_db_info(struct DbInfo* db_info);
    bool add_query_info(struct QueryInfo* query_info);
    bool add_update_info(struct UpdateInfo* update_info);

private:
    bool load_database(const Json::Value& json);
    bool load_query(const Json::Value& json);
    bool load_update(const Json::Value& json);

private:
    sys::DBConnection* init_db_connection(int index) const;

private:
    mutable sys::CReadWriteLock _read_write_lock;
    struct DbInfo* _db_info_array[MAX_DB_CONNECTION];
    struct QueryInfo* _query_info_array[MAX_SQL_TEMPLATE];
    struct UpdateInfo* _update_info_array[MAX_SQL_TEMPLATE];
};

} // namespace mooon
} // namespace db_proxy
#endif // MOOON_DB_PROXY_CONFIG_LOADER_H
