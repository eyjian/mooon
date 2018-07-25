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
#include "sys/report_self.h"
#include "ReportSelfService.h"
#include <inttypes.h>
#include <mooon/net/utils.h>
#include <mooon/net/thrift_helper.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/event.h>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/info.h>
#include <mooon/sys/lock.h>
#include <mooon/sys/log.h>
#include <mooon/sys/mmap.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/md5_helper.h>
#include <mooon/utils/tokener.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#define REPORT_SELF_MODULE_NAME "mooon_report_self"
SYS_NAMESPACE_BEGIN

class CReportSelf
{
public:
    CReportSelf(const std::string& conffile, uint32_t report_interval_seconds);
    ~CReportSelf();
    bool init();
    void stop();
    void run();
    std::pair<uint64_t, uint64_t> get_memory() const { return _mem; }

private:
    bool init_info(); // 初始化本机和当前进程的信息
    bool init_conf(); // 初始化配置，实为读取配置文件
    void report();

private:
    const std::string _conffile;
    const uint32_t _report_interval_seconds;

private:
    volatile bool _stop;
    sys::CLock _lock;
    sys::CEvent _event;

private:
    std::string _ethX;
    std::vector<std::pair<std::string, uint16_t> > _report_servers;
    std::pair<uint64_t, uint64_t> _mem;
    sys::CInfo::process_info_t _process_info;

private:
    uint32_t _pid; // 进程ID
    std::string _ip; // 本机的IP
    std::string _user; // 进程的当前用户名
    std::string _shortname; // 进程的短名字
    std::string _dirpath; // 程序文件所在目录
    std::string _full_cmdline; // 完整的命令行，实际为文件/proc/self/cmdline的内容
    std::string _full_cmdline_md5; // _full_cmdline的MD5值
};

static CReportSelf* g_report_self = NULL;
static sys::CThreadEngine* g_report_self_thread_engine = NULL;

std::pair<uint64_t, uint64_t> get_self_memory()
{
    std::pair<uint64_t, uint64_t> mem(0, 0);
    if (g_report_self != NULL)
        mem = g_report_self->get_memory();
    return mem;
}

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
            MYLOG_INFO("[%s] stop ok:%" PRId64"\n", REPORT_SELF_MODULE_NAME, thread_id);
        }

        delete g_report_self;
        g_report_self = NULL;
    }
}

bool start_report_self(const std::string& conffile, uint32_t report_interval_seconds)
{
    try
    {
        if (g_report_self != NULL)
        {
            return true;
        }
        if (conffile.empty())
        {
            return true;
        }
        if (!CFileUtils::exists(conffile.c_str()))
        {
            return true;
        }

        g_report_self = new CReportSelf(conffile, report_interval_seconds);
        if (!g_report_self->init())
        {
            delete g_report_self;
            g_report_self = NULL;
            return false;
        }

        g_report_self_thread_engine = new mooon::sys::CThreadEngine(mooon::sys::bind(&CReportSelf::run, g_report_self));
        MYLOG_INFO("[%s] start ok: %" PRId64"\n", REPORT_SELF_MODULE_NAME, g_report_self_thread_engine->thread_id());
        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        MYLOG_ERROR("[%s] start failed: %s\n", REPORT_SELF_MODULE_NAME, ex.str().c_str());
        delete g_report_self;
        g_report_self = NULL;
        return false;
    }
}

CReportSelf::CReportSelf(const std::string& conffile, uint32_t report_interval_seconds)
    : _conffile(conffile), _report_interval_seconds(report_interval_seconds+1),
      _stop(false)
{
    _pid = sys::CUtils::get_current_process_id();
    memset(&_process_info, 0, sizeof(_process_info));
}

CReportSelf::~CReportSelf()
{
}

bool CReportSelf::init()
{
    return init_conf() && init_info();
}

void CReportSelf::stop()
{
    sys::LockHelper<sys::CLock> lh(_lock);
    _stop = true;
    _event.signal();
}

void CReportSelf::run()
{
    report(); // 启动时总是上报

    while (!_stop)
    {
        try
        {
            sys::LockHelper<sys::CLock> lh(_lock);
            if (!_event.timed_wait(_lock, _report_interval_seconds*1000))
            {
                report();
            }
        }
        catch (sys::CSyscallException& ex)
        {
            MYLOG_ERROR("[%s] %s\n", REPORT_SELF_MODULE_NAME, ex.str().c_str());
            sys::CUtils::millisleep(1000);
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
            MYLOG_INFO("[%s] config file not exists: %s (%s)\n", REPORT_SELF_MODULE_NAME, _conffile.c_str(), sys::CUtils::get_last_error_message().c_str());
            return true;
        }

        // 正常的配置文件不会很大，4K绰绰有余
        sys::mmap_t* ptr = sys::CMMap::map_read(_conffile.c_str(), SIZE_4K);
        do
        {
            const char* str = static_cast<char*>(ptr->addr);
            rapidjson::Document doc;

            if (doc.Parse(str).HasParseError())
            {
                MYLOG_ERROR("[%s] parse `%s` failed: %d/%zd (%s) in %s\n", REPORT_SELF_MODULE_NAME, str, doc.GetParseError(), doc.GetErrorOffset(), rapidjson::GetParseError_En(doc.GetParseError()), _conffile.c_str());
                break;
            }

            // ethX
            rapidjson::Value::MemberIterator iter = doc.FindMember("ethX");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `ethX` in %s\n", REPORT_SELF_MODULE_NAME, _conffile.c_str());
                break;
            }
            const rapidjson::Value& ethX_value = iter->value;
            _ethX.assign(ethX_value.GetString(), ethX_value.GetStringLength());

            // report_self_servers
            iter = doc.FindMember("report_self_servers");
            if (iter == doc.MemberEnd())
            {
                MYLOG_ERROR("[%s] config error: without `report_self_servers` in %s\n", REPORT_SELF_MODULE_NAME, _conffile.c_str());
                break;
            }
            const rapidjson::Value& report_self_servers_value = iter->value;
            const std::string report_self_servers(report_self_servers_value.GetString(), report_self_servers_value.GetStringLength());

            utils::CEnhancedTokenerEx tokener;
            tokener.parse(report_self_servers, ",", ':');
            const std::multimap<std::string, std::string>& tokens = tokener.tokens();
            if (tokens.empty())
            {
                MYLOG_ERROR("[%s] config error: `report_self_servers` in %s\n", REPORT_SELF_MODULE_NAME, _conffile.c_str());
                break;
            }

            for (std::multimap<std::string, std::string>::const_iterator iter=tokens.begin(); iter!=tokens.end(); ++iter)
            {
                const std::string& report_server_ip_str = iter->first;
                const std::string& report_server_port_str = iter->second;
                const uint16_t report_server_port = utils::CStringUtils::string2int<uint16_t>(report_server_port_str);
                if (report_server_port > 0)
                {
                    _report_servers.push_back(std::make_pair(report_server_ip_str, report_server_port));
                }
            }
            if (_report_servers.empty())
            {
                MYLOG_ERROR("[%s] config error: `report_self_servers` in %s\n", REPORT_SELF_MODULE_NAME, _conffile.c_str());
                break;
            }

            sys::CMMap::unmap(ptr);
            return true;
        } while(false);

        sys::CMMap::unmap(ptr);
        return false;
    }
    catch (sys::CSyscallException& ex)
    {
        MYLOG_ERROR("[%s] init failed: %s\n", REPORT_SELF_MODULE_NAME, ex.str().c_str());
        return false;
    }
}

bool CReportSelf::init_info()
{
    _user = sys::CUtils::get_current_username();
    _shortname = sys::CUtils::get_program_short_name();
    _dirpath = sys::CUtils::get_program_dirpath();
    _full_cmdline = sys::CUtils::get_program_full_cmdline();

    // 取本地IP
    net::string_ip_array_t ip_array;
    net::CUtils::get_ethx_ip(_ethX.c_str(), ip_array);
    if (!ip_array.empty())
        _ip = ip_array[0];
    if (_ip == "127.0.0.1")
        _ip = "";

    _full_cmdline_md5 = utils::CMd5Helper::md5("%s%s%s", _ip.c_str(), _user.c_str(), _full_cmdline.c_str());
    return !_user.empty() && !_ip.empty() &&
           !_shortname.empty() && !_dirpath.empty() &&
           !_full_cmdline.empty() && !_full_cmdline_md5.empty();
}

void CReportSelf::report()
{
    if (init_conf())
    {
        const std::string& current = sys::CDatetimeUtils::get_current_datetime();
        if (!sys::CInfo::get_process_info(&_process_info))
        {
            _mem.first = 0;
            _mem.second = 0;
        }
        else
        {
            _mem.first = _process_info.vsize;
            _mem.second = _process_info.rss * sys::CUtils::get_page_size();
        }

        std::vector<std::string> tokens(11);
        tokens[0] = _full_cmdline_md5;
        tokens[1] = _ip;
        tokens[2] = _user;
        tokens[3] = _shortname;
        tokens[4] = _dirpath;
        //tokens[5] = _full_cmdline; // 包含安全信息，不能上报
        tokens[6] = current;
        tokens[7] = utils::CStringUtils::int_tostring(_report_interval_seconds);
        tokens[8] = utils::CStringUtils::int_tostring(_pid);
        tokens[9] = utils::CStringUtils::int_tostring(_mem.first);
        tokens[10] = utils::CStringUtils::int_tostring(_mem.second);

        for (std::vector<std::pair<std::string, uint16_t> >::size_type i=0; i<_report_servers.size(); ++i)
        {
            const std::pair<std::string, uint16_t>& report_server = _report_servers[i];
            net::CThriftClientHelper<ReportSelfServiceClient> client(report_server.first, report_server.second);

            try
            {
                client.connect();
                client->report(tokens);
                MYLOG_INFO("[%s] report to %s:%d ok\n", REPORT_SELF_MODULE_NAME, report_server.first.c_str(), report_server.second);
                break;
            }
            catch (apache::thrift::transport::TTransportException& ex)
            {
                MYLOG_ERROR("[%s] thrift(%s:%d) transport exception: (%d)%s\n", REPORT_SELF_MODULE_NAME, report_server.first.c_str(), report_server.second, ex.getType(), ex.what());
            }
            catch (apache::thrift::TApplicationException& ex)
            {
                MYLOG_ERROR("[%s] thrift(%s:%d) application exception: %s\n", REPORT_SELF_MODULE_NAME, report_server.first.c_str(), report_server.second, ex.what());
            }
            catch (apache::thrift::TException& ex)
            {
                MYLOG_ERROR("[%s] thrift(%s:%d) exception: %s\n", REPORT_SELF_MODULE_NAME, report_server.first.c_str(), report_server.second, ex.what());
            }
        }
    }
}

SYS_NAMESPACE_END
