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
// 下载工具
// 使用示例：
// ./mooon_download -h=192.168.10.11 -p=6000 -u=root -P='root123' -s=/etc/hosts,/etc/passwd -d=/tmp/
// 表示将192.168.10.11机器上的文件/etc/hosts和/etc/passwd两个文件下载到本地的/tmp目录下
//
// 可环境变量H替代参数“-h”
// 可环境变量U替代参数“-u”
// 可环境变量P替代参数“-p”
#include "mooon/net/libssh2.h"
#include "mooon/sys/stop_watch.h"
#include "mooon/utils/args_parser.h"
#include "mooon/utils/print_color.h"
#include "mooon/utils/string_utils.h"
#include "mooon/utils/tokener.h"
#include <fstream>
#include <iostream>

// 逗号分隔的远程主机IP列表
STRING_ARG_DEFINE(h, "", "Connect to the remote machines on the given hosts separated by comma, can be replaced by environment variable 'H', example: -h='192.168.1.10,192.168.1.11'");
// 远程主机的sshd端口号
INTEGER_ARG_DEFINE(uint16_t, P, 22, 10, 65535, "Specifies the port to connect to on the remote machines, can be replaced by environment variable 'PORT'");
// 用户名
STRING_ARG_DEFINE(u, "", "Specifies the user to log in as on the remote machines, can be replaced by environment variable 'U'");
// 密码
STRING_ARG_DEFINE(p, "", "The password to use when connecting to the remote machines, can be replaced by environment variable 'P'");
// 连接超时，单位为秒
INTEGER_ARG_DEFINE(uint16_t, t, 60, 1, 65535, "The number of seconds before connection timeout");

// 被上传的文件路径
STRING_ARG_DEFINE(s, "", "The remote source files downloaded, separated by comma, example: -s='/tmp/x1.txt,/tmp/x2.txt'");
// 文件上传后存放的目录路径
STRING_ARG_DEFINE(d, "", "The local destination directory, example: -d='/tmp/'");

// 结果信息
struct ResultInfo
{
    bool success; // 为true表示执行成功
    std::string ip; // 远程host的IP地址
    std::string source; // 被下载的文件
    uint32_t seconds; // 运行花费的时长，精确到秒

    ResultInfo()
        : success(false), seconds(0)
    {
    }

    std::string str() const
    {
        std::string tag = success? "SUCCESS": "FAILURE";
        return mooon::utils::CStringUtils::format_string("[%s %s]: %u seconds (%s)", ip.c_str(), tag.c_str(), seconds, source.c_str());
    }
};

inline std::ostream& operator <<(std::ostream& out, const struct ResultInfo& result)
{
    std::string tag = result.success? "SUCCESS": "FAILURE";
    out << "[" PRINT_COLOR_YELLOW << result.ip << PRINT_COLOR_NONE" " << tag << "] " << result.seconds << " seconds (" << result.source << ")";
    return out;
}

// 可环境变量H替代参数“-h”
// 可环境变量U替代参数“-u”
// 可环境变量P替代参数“-p”
int main(int argc, char* argv[])
{
#if MOOON_HAVE_LIBSSH2 == 1
    // 解析命令行参数
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        if (errmsg.empty())
        {
            fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        }
        else
        {
            fprintf(stderr, "parameter error: %s\n\n", errmsg.c_str());
            fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        }
        exit(1);
    }

    uint16_t port = mooon::argument::P->value();
    std::string sources = mooon::argument::s->value();
    std::string directory = mooon::argument::d->value();
    std::string host = mooon::argument::h->value();
    std::string user = mooon::argument::u->value();
    std::string password = mooon::argument::p->value();
    mooon::utils::CStringUtils::trim(sources);
    mooon::utils::CStringUtils::trim(directory);
    mooon::utils::CStringUtils::trim(host);
    mooon::utils::CStringUtils::trim(user);
    mooon::utils::CStringUtils::trim(password);

    // 检查参数（-P）
    const char* port_ = getenv("PORT");
    if (port_ != NULL)
    {
        // 优先使用环境变量的值，但如果不是合法的值，则仍然使用参数值
        (void)mooon::utils::CStringUtils::string2int(port_, port);
    }

    // 检查参数（-s）
    if (sources.empty())
    {
        fprintf(stderr, "parameter[-s]'s value not set\n\n");
        fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
        exit(1);
    }

    // 检查参数（-d）
    if (directory.empty())
    {
        fprintf(stderr, "parameter[-d]'s value not set\n\n");
        fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
        exit(1);
    }

    // 检查参数（-h）
    if (host.empty())
    {
        // 尝试从环境变量取值
        const char* host_ = getenv("H");
        if (NULL == host_)
        {
            fprintf(stderr, "parameter[-h]'s or environment `H` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }

        host = host_;
        mooon::utils::CStringUtils::trim(host);
        if (host.empty())
        {
            fprintf(stderr, "parameter[-h] or environment `H` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }
    }

    // 检查参数（-u）
    if (user.empty())
    {
        // 尝试从环境变量取值
        const char* user_ = getenv("U");
        if (NULL == user_)
        {
            fprintf(stderr, "parameter[-u] or environment `U` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }

        user= user_;
        mooon::utils::CStringUtils::trim(user);
        if (user.empty())
        {
            fprintf(stderr, "parameter[-u] or environment `U` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }
    }

    // 检查参数（-p）
    if (password.empty())
    {
        // 尝试从环境变量取值
        const char* password_ = getenv("P");
        if (NULL == password_)
        {
            fprintf(stderr, "parameter[-p] or environment `P` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }

        password= password_;
        if (password.empty())
        {
            fprintf(stderr, "parameter[-p] or environment `P` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }
    }

    std::vector<std::string> source_files;
    const int num_source_files = mooon::utils::CTokener::split(&source_files, sources, ",", true);

    mooon::net::CLibssh2::init();
    std::vector<struct ResultInfo> results(num_source_files);
    for (int j=0; j<num_source_files; ++j)
    {
        bool color = true;

        std::string local_filepath = directory + std::string("/") + mooon::utils::CStringUtils::extract_filename(source_files[j]);
        results[j].ip = host;
        results[j].source = source_files[j];
        results[j].success = false;

        std::ofstream local_fs(local_filepath.c_str());
        mooon::sys::CStopWatch stop_watch;
        try
        {
            int file_size = 0;
            mooon::net::CLibssh2 libssh2(host, port, user, password, mooon::argument::t->value());
            libssh2.download(source_files[j], local_fs, &file_size);

            fprintf(stdout, "[" PRINT_COLOR_YELLOW"%s" PRINT_COLOR_NONE"] SUCCESS: %d bytes (%s)\n", host.c_str(), file_size, source_files[j].c_str());
            results[j].success = true;
        }
        catch (mooon::sys::CSyscallException& ex)
        {
            if (color)
                fprintf(stdout, PRINT_COLOR_NONE); // color = true;

            fprintf(stderr, "[" PRINT_COLOR_RED"%s" PRINT_COLOR_NONE"] failed: %s (%s)\n", host.c_str(), ex.str().c_str(), source_files[j].c_str());
        }
        catch (mooon::utils::CException& ex)
        {
            if (color)
                fprintf(stdout, PRINT_COLOR_NONE); // color = true;

            fprintf(stderr, "[" PRINT_COLOR_RED"%s" PRINT_COLOR_NONE"] failed: %s (%s)\n", host.c_str(), ex.str().c_str(), source_files[j].c_str());
        }

        results[j].seconds = stop_watch.get_elapsed_microseconds() / 1000000;
    }
    mooon::net::CLibssh2::fini();

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
#else
    fprintf(stderr, "NOT IMPLEMENT! please install libssh2 (https://www.libssh2.org/) into /usr/local/libssh2 and recompile.\n");
#endif // MOOON_HAVE_LIBSSH2 == 1

    return (0 == num_failure)? 0: 1;
}
