/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: jian yi, eyjian@qq.com
 */
#include "mooon/sys/report_self.h"
#include <inttypes.h>
#include <mooon/net/utils.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/event.h>
#include <mooon/sys/info.h>
#include <mooon/sys/lock.h>
#include <mooon/sys/log.h>
#include <mooon/sys/mmap.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/md5_helper.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#if MOOON_HAVE_MYSQL==1

#define REPORT_SELF_MODULE_NAME "ReportSelf"
SYS_NAMESPACE_BEGIN

class CReportSelf
{
public:
    CReportSelf(const std::string& conffile, uint32_t report_interval_seconds);
    ~CReportSelf();
    bool init();
    void stop();
    void run();

private:
    bool init_info(); // 初始化本机和当前进程的信息
    bool init_conf(); // 初始化配置，实为读取配置文件
    bool init_mysql(CMySQLConnection* mysql);
    void report();

private:
    const std::string _conffile;
    const uint32_t _report_interval_seconds;

private:
    volatile bool _stop;
    CLock _lock;
    CEvent _event;

private:
    std::string _ethX;
    std::string _db_ip;
    uint16_t _db_port;
    std::string _db_user;
    std::string _db_password;
    std::string _db_name;
    std::string _table_name;

private:
    uint32_t _pid;
    std::string _ip; // 本机的IP
    std::string _user; // 进程的当前用户名
    std::string _shortname; // 进程的短名字
    std::string _dirpath; // 程序文件所在目录
    std::string _full_cmdline; // 完整的命令行，实际为文件/proc/self/cmdline的内容
    std::string _full_cmdline_md5; // _full_cmdline的MD5值
};

static CReportSelf* g_report_self = NULL;
static CThreadEngine* g_report_self_thread_engine = NULL;

void stop_report_self()
{
    if (g_report_self != NULL)
    {
        g_report_self->stop();

        if (g_report_self_thread_engine != NULL)
        {
            const uint64_t thread_id = g_report_self_thread_engine->thread_id();
            g_report_self_thread_engine->join();
            delete g_report_self_thread_engine;
            g_report_self_thread_engine = NULL;
            MYLOG_ERROR("[%s] stop ok:%" PRId64"\n", REPORT_SELF_MODULE_NAME, thread_id);
        }

        delete g_report_self;
        g_report_self = NULL;
    }
}

bool start_report_self(const std::string& conffile, uint32_t report_interval_seconds)
{
    if (!conffile.empty())
    {
        if (NULL == g_report_self)
        {
            g_report_self = new CReportSelf(conffile, report_interval_seconds);
            if (!g_report_self->init())
            {
                delete g_report_self;
                g_report_self = NULL;
                return false;
            }

            try
            {
                g_report_self_thread_engine = new mooon::sys::CThreadEngine(mooon::sys::bind(&CReportSelf::run, g_report_self));
                MYLOG_INFO("[%s] start ok: %" PRId64"\n", REPORT_SELF_MODULE_NAME, g_report_self_thread_engine->thread_id());
            }
            catch (CSyscallException& ex)
            {
                MYLOG_ERROR("[%s] start failed: %s\n", REPORT_SELF_MODULE_NAME, ex.str().c_str());
                delete g_report_self;
                g_report_self = NULL;
                return false;
            }
        }
    }

    return true;
}

CReportSelf::CReportSelf(const std::string& conffile, uint32_t report_interval_seconds)
    : _conffile(conffile), _report_interval_seconds(report_interval_seconds+1),
      _stop(false)
{
    _pid = CUtils::get_current_process_id();
}

CReportSelf::~CReportSelf()
{
}

bool CReportSelf::init()
{
    CMySQLConnection mysql;
    return init_conf() && init_info() && init_mysql(&mysql);
}

void CReportSelf::stop()
{
    _stop = false;
}

void CReportSelf::run()
{
    report(); // 启动时总是上报

    while (!_stop)
    {
        try
        {
            LockHelper<CLock> lh(_lock);
            if (!_event.timed_wait(_lock, _report_interval_seconds*1000))
            {
                report();
            }
        }
        catch (CSyscallException& ex)
        {
            MYLOG_ERROR("[%s] %s\n", REPORT_SELF_MODULE_NAME, ex.str().c_str());
            CUtils::millisleep(1000);
        }
    }
}

bool CReportSelf::init_conf()
{
    try
    {
        // 如果不指定配置文件，则认为不想启用
        if (_conffile.empty())
        {
            MYLOG_INFO("[%s] not given config file\n", REPORT_SELF_MODULE_NAME);
            return true;
        }

        // 如果配置文件不存在或无读取权限，则也认为不想启用
        if (access(_conffile.c_str(), R_OK) != 0)
        {
            MYLOG_INFO("[%s] config file not exists: %s (%s)\n", REPORT_SELF_MODULE_NAME, _conffile.c_str(), CUtils::get_last_error_message().c_str());
            return true;
        }

        // 正常的配置文件不会很大，4K绰绰有余
        mmap_t* ptr = CMMap::map_read(_conffile.c_str(), SIZE_4K);
        do
        {
            const char* str = static_cast<char*>(ptr->addr);
            rapidjson::Document doc;

            if (doc.Parse(str).HasParseError())
            {
                MYLOG_ERROR("[%s] parse `%s` failed: %d/%zd (%s)\n", REPORT_SELF_MODULE_NAME, str, doc.GetParseError(), doc.GetErrorOffset(), rapidjson::GetParseError_En(doc.GetParseError()));
                break;
            }

            // ethX
            rapidjson::Value::MemberIterator iter = doc.FindMember("ethX");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `ethX`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& ethX_value = iter->value;
            _ethX.assign(ethX_value.GetString(), ethX_value.GetStringLength());

            // db_ip
            iter = doc.FindMember("db_ip");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `db_ip`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& db_ip_value = iter->value;
            _db_ip.assign(db_ip_value.GetString(), db_ip_value.GetStringLength());

            // db_port
            iter = doc.FindMember("db_port");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `db_port`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& db_port_value = iter->value;
            _db_port = static_cast<uint16_t>(db_port_value.GetInt());

            // db_user
            iter = doc.FindMember("db_user");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `db_user`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& db_user_value = iter->value;
            _db_user.assign(db_user_value.GetString(), db_user_value.GetStringLength());

            // db_password
            iter = doc.FindMember("db_password");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `db_password`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& db_password_value = iter->value;
            _db_password.assign(db_password_value.GetString(), db_password_value.GetStringLength());

            // db_name
            iter = doc.FindMember("db_name");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `db_name`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& db_name_value = iter->value;
            _db_name.assign(db_name_value.GetString(), db_name_value.GetStringLength());

            // table_name
            iter = doc.FindMember("table_name");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `table_name`\n", REPORT_SELF_MODULE_NAME);
                break;
            }
            const rapidjson::Value& table_name_value = iter->value;
            _table_name.assign(table_name_value.GetString(), table_name_value.GetStringLength());

            CMMap::unmap(ptr);
            return true;
        } while(false);

        CMMap::unmap(ptr);
        return false;
    }
    catch (CSyscallException& ex)
    {
        MYLOG_ERROR("[%s] init failed: %s\n", REPORT_SELF_MODULE_NAME, ex.str().c_str());
        return false;
    }
}

bool CReportSelf::init_info()
{
    _user = CUtils::get_current_username();
    _shortname = CUtils::get_program_short_name();
    _dirpath = CUtils::get_program_dirpath();
    _full_cmdline = CUtils::get_program_full_cmdline();
    _full_cmdline_md5 = utils::CMd5Helper::md5("%s", _full_cmdline.c_str());

    // 取本地IP
    net::string_ip_array_t ip_array;
    net::CUtils::get_ethx_ip(_ethX.c_str(), ip_array);
    if (!ip_array.empty())
        _ip = ip_array[0];
    if (_ip == "127.0.0.1")
        _ip = "";

    return !_user.empty() && !_ip.empty() &&
           !_shortname.empty() && !_dirpath.empty() &&
           !_full_cmdline.empty() && !_full_cmdline_md5.empty();
}

bool CReportSelf::init_mysql(CMySQLConnection* mysql)
{
    try
    {
        mysql->set_host(_db_ip, _db_port);
        mysql->set_user(_db_user, _db_password);
        mysql->set_db_name(_db_name);
        mysql->enable_auto_reconnect(true);
        mysql->open();
        return true;
    }
    catch (CDBException& ex)
    {
        MYLOG_ERROR("[%s] update %s error: %s\n", REPORT_SELF_MODULE_NAME, mysql->str().c_str(), ex.str().c_str());
        return false;
    }
}

void CReportSelf::report()
{
    CMySQLConnection mysql; // 上报频率低，短连接即可

    if (init_conf() && init_mysql(&mysql))
    {
        uint64_t vsz = 0;
        uint64_t vss = 0;
        CInfo::process_info_t process_info;
        if (CInfo::get_process_info(process_info))
        {
            vsz = process_info.vsize;
            vss = process_info.rss * CUtils::get_page_size();
        }

        try
        {
            const std::string& current = CDatetimeUtils::get_current_datetime();
            const std::string& update_sql = utils::CStringUtils::format_string(
                    "UPDATE %s SET f_ip='%s',f_user='%s',f_shortname='%s',f_dirpath='%s',f_full_cmdline='%s',f_lasttime='%s',f_interval=%u,f_pid=%u,f_vsz=%" PRIu64",f_vss=%" PRIu64" WHERE f_md5='%s'",
                    _table_name.c_str(),
                    _ip.c_str(), _user.c_str(), _shortname.c_str(), _dirpath.c_str(), _full_cmdline.c_str(), current.c_str(), _report_interval_seconds, _pid, vsz, vss,
                    _full_cmdline_md5.c_str());

            const uint64_t effected_rows = mysql.update("%s", update_sql.c_str());
            if (effected_rows > 0)
            {
                MYLOG_INFO("[%s][%s] %s\n", REPORT_SELF_MODULE_NAME, mysql.str().c_str(), update_sql.c_str());
            }
            else
            {
                const std::string& insert_sql = utils::CStringUtils::format_string(
                        "INSERT INTO %s (f_md5,f_ip,f_user,f_shortname,f_dirpath,f_full_cmdline,f_lasttime,f_interval,f_pid,f_vsz,f_vss) VALUES ('%s','%s','%s','%s','%s','%s','%s',%u,%u,%" PRIu64",%" PRIu64")",
                        _table_name.c_str(), _full_cmdline_md5.c_str(),
                        _ip.c_str(), _user.c_str(), _shortname.c_str(), _dirpath.c_str(), _full_cmdline.c_str(), current.c_str(), _report_interval_seconds, _pid, vsz, vss);
                mysql.update("%s", insert_sql.c_str());
                MYLOG_INFO("[%s][%s] %s\n", REPORT_SELF_MODULE_NAME, mysql.str().c_str(), insert_sql.c_str());
            }
        }
        catch (CDBException& ex)
        {
            MYLOG_ERROR("[%s] update %s error: %s\n", REPORT_SELF_MODULE_NAME, mysql.str().c_str(), ex.str().c_str());
        }
    }
}

SYS_NAMESPACE_END

#endif // MOOON_HAVE_MYSQL
