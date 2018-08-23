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
//
// 可环境变量HOSTS替代参数“-h”
// 可环境变量USER替代参数“-u”
// 可环境变量PASSWORD替代参数“-p”
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
STRING_ARG_DEFINE(s, "", "The local source files uploaded, separated by comma, example: -s='/tmp/x1.txt,/tmp/x2.txt'");
// 文件上传后存放的目录路径
STRING_ARG_DEFINE(d, "", "The remote destination directory, example: -d='/tmp/'");

// 结果信息
struct ResultInfo
{
    bool success; // 为true表示执行成功
    std::string ip; // 远程host的IP地址
    std::string source; // 被上传的文件
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
    std::string hosts = mooon::argument::h->value();
    std::string user = mooon::argument::u->value();
    std::string password = mooon::argument::p->value();
    mooon::utils::CStringUtils::trim(sources);
    mooon::utils::CStringUtils::trim(directory);
    mooon::utils::CStringUtils::trim(hosts);
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
    if (hosts.empty())
    {
        // 尝试从环境变量取值
        const char* hosts_ = getenv("H");
        if (NULL == hosts_)
        {
            fprintf(stderr, "parameter[-h] or environment `H` not set\n\n");
            fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
            exit(1);
        }

        hosts = hosts_;
        mooon::utils::CStringUtils::trim(hosts);
        if (hosts.empty())
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
            fprintf(stderr, "parameter[-u] or environment `U` not set\n");
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
    int num_source_files = mooon::utils::CTokener::split(&source_files, sources, ",", true);

    std::vector<std::string> hosts_ip;
    const std::string& remote_hosts_ip = hosts;
    int num_remote_hosts_ip = mooon::utils::CTokener::split(&hosts_ip, remote_hosts_ip, ",", true);
    if (0 == num_remote_hosts_ip)
    {
    	fprintf(stderr, "parameter[-h] or environment `H` error\n\n");
    	fprintf(stderr, "%s\n", mooon::utils::CArgumentContainer::get_singleton()->usage_string().c_str());
        exit(1);
    }

    mooon::net::CLibssh2::init();
    std::vector<struct ResultInfo> results(num_remote_hosts_ip * num_source_files);
    for (int i=0, k=0; i<num_remote_hosts_ip; ++i)
    {
    	for (int j=0; j<num_source_files; ++j)
    	{
			bool color = true;

			std::string remote_filepath = directory + std::string("/") + mooon::utils::CStringUtils::extract_filename(source_files[j]);
			const std::string& remote_host_ip = hosts_ip[i];
			results[k].ip = remote_host_ip;
			results[k].source = source_files[j];
			results[k].success = false;

			fprintf(stdout, "[" PRINT_COLOR_YELLOW"%s" PRINT_COLOR_NONE"]\n", remote_host_ip.c_str());
			fprintf(stdout, PRINT_COLOR_GREEN);

			mooon::sys::CStopWatch stop_watch;
			try
			{
				int file_size = 0;
				mooon::net::CLibssh2 libssh2(remote_host_ip, port, user, password, mooon::argument::t->value());
				libssh2.upload(source_files[j], remote_filepath, &file_size);

				fprintf(stdout, "[" PRINT_COLOR_YELLOW"%s" PRINT_COLOR_NONE"] SUCCESS: %d bytes (%s)\n", remote_host_ip.c_str(), file_size, source_files[j].c_str());
				results[k].success = true;
			}
			catch (mooon::sys::CSyscallException& ex)
			{
				if (color)
					fprintf(stdout, PRINT_COLOR_NONE); // color = true;

				fprintf(stderr, "[" PRINT_COLOR_RED"%s" PRINT_COLOR_NONE"] failed: %s (%s)\n", remote_host_ip.c_str(), ex.str().c_str(), source_files[j].c_str());
			}
			catch (mooon::utils::CException& ex)
			{
				if (color)
					fprintf(stdout, PRINT_COLOR_NONE); // color = true;

				fprintf(stderr, "[" PRINT_COLOR_RED"%s" PRINT_COLOR_NONE"] failed: %s (%s)\n", remote_host_ip.c_str(), ex.str().c_str(), source_files[j].c_str());
			}

			std::cout << std::endl;
			results[k].seconds = stop_watch.get_elapsed_microseconds() / 1000000;
			++k;
    	}
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
