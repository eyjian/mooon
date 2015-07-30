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
#include "mooon/net/libssh2.h"
#include "mooon/utils/tokener.h"
MOOON_NAMESPACE_USE

// 示例：
// ./ut_libssh2 tom@127.0.0.1:22#tom2015 whoami
int main(int argc, char* argv[])
{
#if HAVE_LIBSSH2 == 1
    net::CLibssh2::init();
    if (argc != 3)
    {
        fprintf(stderr, "usage: ut_libssh2 username@ip:port#password command\n");
        exit(1);
    }

    std::string login = argv[1];
    std::string command = argv[2];

    std::vector<struct utils::CLoginTokener::LoginInfo> login_infos;
    int num = utils::CLoginTokener::parse(&login_infos, login, ",");

    if (-1 == num)
    {
        fprintf(stderr, "invalid paramter: %s\n", login.c_str());
        fprintf(stderr, "usage: ut_libssh2 username@ip:port#password command\n");
        exit(1);
    }
    for (int i=0; i<num; ++i)
    {
        try
        {
            int exitcode;
            std::string exitsignal;
            std::string errmsg;
            int num_bytes;
            const utils::CLoginTokener::LoginInfo& login_info = login_infos[i];

            printf("ip[%s], port[%u], username[%s], password[%s]\n\n",
                login_info.ip.c_str(), login_info.port, login_info.username.c_str(), login_info.password.c_str());
            net::CLibssh2 libssh2(login_info.ip, login_info.port, login_info.username, login_info.password);

            // 远程执行命令
            libssh2.remotely_execute(command, std::cout, &exitcode, &exitsignal, &errmsg, &num_bytes);
            printf("\nexitcode=%d(%s), exitsignal=%s, errmsg=%s, num_bytes=%d\n", exitcode, strerror(exitcode), exitsignal.c_str(), errmsg.c_str(), num_bytes);

            // 下载文件
            libssh2.download("/etc/hosts", std::cout, &num_bytes);
            printf("num_bytes: %d\n", num_bytes);

            // 上传文件
            libssh2.upload("/etc/hosts", "/tmp/hosts", &num_bytes);
            printf("num_bytes: %d\n", num_bytes);
        }
        catch (sys::CSyscallException& syscall_ex)
        {
            fprintf(stderr, "%s\n", syscall_ex.str().c_str());
        }
        catch (utils::CException& ex)
        {
            fprintf(stderr, "%s\n", ex.str().c_str());
        }
    }

#endif // HAVE_LIBSSH2
    return 0;
}

