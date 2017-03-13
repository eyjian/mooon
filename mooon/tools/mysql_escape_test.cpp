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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#if HAVE_MYSQL==1
#include <mysql/mysql.h>
#include <mooon/sys/mysql_db.h>
#endif // HAVE_MYSQL
#include <mooon/utils/tokener.h>

// 参数1：DB连接字符串
// 参数2：需要转义的字符串
int main(int argc, char* argv[])
{
#if HAVE_MYSQL==1
    printf("MYSQL_SERVER_VERSION: %s\n", MYSQL_SERVER_VERSION);

    if ((argc != 3) && (argc != 4))
    {
        fprintf(stderr, "usage1: mysql_escape_test 'username@IP:port#password' 'string'\n");
        fprintf(stderr, "usage2: mysql_escape_test 'username@IP:port#password' 'string' 'charset'\n");
        exit(1);
    }

    std::vector<struct mooon::utils::CLoginTokener::LoginInfo> login_infos;
    if (mooon::utils::CLoginTokener::parse(&login_infos, argv[1], "") != 1)
    {
        fprintf(stderr, "usage: mysql_escape_test 'username@IP:port#password' 'string'\n");
        exit(1);
    }

    try
    {
        mooon::sys::CMySQLConnection mysql;
        const struct mooon::utils::CLoginTokener::LoginInfo& login_info = login_infos[0];
        mysql.set_host(login_info.ip, login_info.port);
        mysql.set_user(login_info.username, login_info.password);
        if (4 == argc)
            mysql.set_charset(argv[3]);

        mysql.open();
        printf("%s\n", mysql.escape_string(argv[2]).c_str());
    }
    catch (mooon::sys::CDBException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }
#endif // HAVE_MYSQL
    return 0;
}
