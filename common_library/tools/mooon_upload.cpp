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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
// 批量上传工具
// 使用示例：
// ./mooon_upload -h=192.168.10.11,192.168.10.12 -p=6000 -u=root -P='root123' -s=./abc -d=/tmp/
// 表示将本地的文件./abc上传到两台机器192.168.10.11和192.168.10.12的/tmp/目录
#include "mooon/net/libssh2.h"
#include "mooon/sys/stop_watch.h"
#include "mooon/utils/args_parser.h"
#include "mooon/utils/print_color.h"
#include "mooon/utils/string_utils.h"
#include "mooon/utils/tokener.h"
#include <fstream>
#include <iostream>

// 逗号分隔的远程主机列表
STRING_ARG_DEFINE(h, "", "remote hosts");
// 远程主机的sshd端口号
INTEGER_ARG_DEFINE(uint16_t, p, 36000, 10, 65535, "remote hosts port");
// 用户名
STRING_ARG_DEFINE(u, "root", "remote host user");
// 密码
STRING_ARG_DEFINE(P, "", "remote host password");

// 被上传的文件路径
STRING_ARG_DEFINE(s, "", "the source file uploaded");
// 文件上传后存放的目录路径
STRING_ARG_DEFINE(d, "", "the directory to store");

// 连接超时，单位为秒
INTEGER_ARG_DEFINE(uint16_t, t, 10, 1, 65535, "timeout seconds to remote host");

// 结果信息
struct ResultInfo
{
    bool success; // 为true表示执行成功
    std::string ip; // 远程host的IP地址
    uint32_t seconds; // 运行花费的时长，精确到秒

    ResultInfo()
        : success(false), seconds(0)
    {
    }

    std::string str() const
    {
        std::string tag = success? "SUCCESS": "FAILURE";
        return mooon::utils::CStringUtils::format_string("[%s %s]: %u seconds", ip.c_str(), tag.c_str(), seconds);
    }
};

inline std::ostream& operator <<(std::ostream& out, const struct ResultInfo& result)
{
    std::string tag = result.success? "SUCCESS": "FAILURE";
    out << "["PRINT_COLOR_YELLOW << result.ip << PRINT_COLOR_NONE" " << tag << "] " << result.seconds << " seconds";
    return out;
}

int main(int argc, char* argv[])
{
    // 解析命令行参数
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "parameter error: %s\n", errmsg.c_str());
        exit(1);
    }

    uint16_t port = mooon::argument::p->value();
    std::string source = mooon::argument::s->value();
    std::string directory = mooon::argument::d->value();
    std::string hosts = mooon::argument::h->value();
    std::string user = mooon::argument::u->value();
    std::string password = mooon::argument::P->value();
    mooon::utils::CStringUtils::trim(source);
    mooon::utils::CStringUtils::trim(directory);
    mooon::utils::CStringUtils::trim(hosts);
    mooon::utils::CStringUtils::trim(user);
    mooon::utils::CStringUtils::trim(password);

    // 检查参数（-s）
    if (hosts.empty())
    {
        fprintf(stderr, "parameter[-s]'s value not set\n");
        exit(1);
    }

    // 检查参数（-d）
    if (hosts.empty())
    {
        fprintf(stderr, "parameter[-d]'s value not set\n");
        exit(1);
    }

    // 检查参数（-h）
    if (hosts.empty())
    {
        // 尝试从环境变量取值
        const char* hosts_ = getenv("HOSTS");
        if (NULL == hosts_)
        {
            fprintf(stderr, "parameter[-h]'s value not set\n");
            exit(1);
        }

        hosts= hosts_;
        mooon::utils::CStringUtils::trim(hosts);
        if (hosts.empty())
        {
            fprintf(stderr, "parameter[-h]'s value not set\n");
            exit(1);
        }
    }

    // 检查参数（-u）
    if (user.empty())
    {
        fprintf(stderr, "parameter[-u]'s value not set\n");
        exit(1);
    }

    // 检查参数（-P）
    if (password.empty())
    {
        fprintf(stderr, "parameter[-P]'s value not set\n");
        exit(1);
    }

    std::vector<std::string> hosts_ip;
    const std::string& remote_hosts_ip = hosts;
    int num_remote_hosts_ip = mooon::utils::CTokener::split(&hosts_ip, remote_hosts_ip, ",", true);
    if (0 == num_remote_hosts_ip)
    {
        fprintf(stderr, "parameter[-h] error\n");
        exit(1);
    }

    std::string remote_filepath = directory + std::string("/") + mooon::utils::CStringUtils::extract_filename(source);
    std::vector<struct ResultInfo> results(num_remote_hosts_ip);
    for (int i=0; i<num_remote_hosts_ip; ++i)
    {
        bool color = true;
        const std::string& remote_host_ip = hosts_ip[i];
        results[i].ip = remote_host_ip;
        results[i].success = false;

        fprintf(stdout, "["PRINT_COLOR_YELLOW"%s"PRINT_COLOR_NONE"]\n", remote_host_ip.c_str());
        fprintf(stdout, PRINT_COLOR_GREEN);

        mooon::sys::CStopWatch stop_watch;
        try
        {
            int file_size = 0;
            mooon::net::CLibssh2 libssh2(remote_host_ip, port, user, password, mooon::argument::t->value());
            libssh2.upload(source, remote_filepath, &file_size);

            fprintf(stdout, "["PRINT_COLOR_YELLOW"%s"PRINT_COLOR_NONE"] SUCCESS: %d bytes\n", remote_host_ip.c_str(), file_size);
            results[i].success = true;
        }
        catch (mooon::sys::CSyscallException& ex)
        {
            if (color)
                fprintf(stdout, PRINT_COLOR_NONE); // color = true;

            fprintf(stderr, "["PRINT_COLOR_RED"%s"PRINT_COLOR_NONE"] failed: %s\n", remote_host_ip.c_str(), ex.str().c_str());
        }
        catch (mooon::utils::CException& ex)
        {
            if (color)
                fprintf(stdout, PRINT_COLOR_NONE); // color = true;

            fprintf(stderr, "["PRINT_COLOR_RED"%s"PRINT_COLOR_NONE"] failed: %s\n", remote_host_ip.c_str(), ex.str().c_str());
        }

        results[i].seconds = stop_watch.get_elapsed_microseconds() / 1000000;
        std::cout << std::endl;
    }

    // 输出总结
    std::cout << std::endl;
    std::cout << "================================" << std::endl;
    int num_success = 0; // 成功的个数
    int num_failure = 0; // 失败的个数
    for (std::vector<struct ResultInfo>::size_type i=0; i<results.size(); ++i)
    {
        const struct ResultInfo& result_info = results[i];
        std::cout << result_info << std::endl;

        if (result_info.success)
            ++num_success;
        else
            ++num_failure;
    }
    std::cout << "SUCCESS: " << num_success << ", FAILURE: " << num_failure << std::endl;

    return 0;
}
