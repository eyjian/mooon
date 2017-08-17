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
#if MOOON_HAVE_MYSQL==1
#include <mysql/mysql.h>
#include <mooon/sys/mysql_db.h>
#endif // MOOON_HAVE_MYSQL
#include <mooon/utils/tokener.h>

static void usage()
{
    fprintf(stderr, "usage1: mysql_escape_test 'username@IP:port#password' 'sql'\n");
    fprintf(stderr, "usage2: mysql_escape_test 'username@IP:port#password' 'sql' 'charset'\n");
}

static std::string escape_string(MYSQL* mysql, const std::string& src)
{
#if MOOON_HAVE_MYSQL==1
#if MYSQL_VERSION_ID >= 50706
    unsigned long n = 0;
    std::string from = src;
    std::string to(from.size()*2+1, '\0');

    // \'
    n = mysql_real_escape_string_quote(mysql, const_cast<char*>(to.c_str()), from.c_str(), static_cast<unsigned long>(from.size()),'\'');
    if (n < 0)
    {
        to.resize(0);
    }
    else
    {
        to.resize(n);
        fprintf(stdout, "['] %s\n", to.c_str());
    }

    // \"
    from = to;
    to.resize(from.size()*2+1, '\0');
    n = mysql_real_escape_string_quote(mysql, const_cast<char*>(to.c_str()), from.c_str(), static_cast<unsigned long>(from.size()),'"');
    if (n < 0)
    {
        to.resize(0);
    }
    else
    {
        to.resize(n);
        fprintf(stdout, "[\"] %s\n", to.c_str());
    }

    from = to;
    to.resize(from.size()*2+1, '\0');
    n = mysql_real_escape_string_quote(mysql, const_cast<char*>(to.c_str()), from.c_str(), static_cast<unsigned long>(from.size()),'\\');
    if (n < 0)
    {
        to.resize(0);
    }
    else
    {
        to.resize(n);
        fprintf(stdout, "[\\] %s\n", to.c_str());
    }

    return to;
#endif // MYSQL_VERSION_ID
#endif // MOOON_HAVE_MYSQL

    return src;
}

// 参数1：DB连接字符串
// 参数2：需要转义的字符串
//
// 示例：
// mysql_escape_test 'root@127.0.0.1' 'SELECT Host,User FROM mysql.user WHERE User=root'
int main(int argc, char* argv[])
{
#if MOOON_HAVE_MYSQL==1
    printf("MYSQL_SERVER_VERSION: %s\n", MYSQL_SERVER_VERSION);

    if ((argc != 3) && (argc != 4))
    {
        usage();
        exit(1);
    }

    std::vector<struct mooon::utils::CLoginTokener::LoginInfo> login_infos;
    if (mooon::utils::CLoginTokener::parse(&login_infos, argv[1], "") != 1)
    {
        fprintf(stderr, "invalid first parameter: %s\n", argv[1]);
        usage();
        exit(1);
    }


    mooon::sys::DBTable dbtable;
    mooon::sys::CMySQLConnection mysql;
    const struct mooon::utils::CLoginTokener::LoginInfo& login_info = login_infos[0];
    mysql.set_host(login_info.ip, login_info.port);
    mysql.set_user(login_info.username, login_info.password);
    if (4 == argc)
        mysql.set_charset(argv[3]);

    try
    {
        mysql.open();
        fprintf(stdout, "[1] %s\n", mysql.escape_string(argv[2]).c_str());

        mysql.update("%s", "SET sql_mode='NO_BACKSLASH_ESCAPES'");
        fprintf(stdout, "[2] %s\n", mysql.escape_string(argv[2]).c_str());
    }
    catch (mooon::sys::CDBException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }

    try
    {
        MYSQL* mysql_handle = static_cast<MYSQL*>(mysql.get_mysql_handle());
        fprintf(stdout, "[3] %s\n", escape_string(mysql_handle, argv[2]).c_str());
        fprintf(stdout, "\n");

        mysql.query(dbtable, "%s", escape_string(mysql_handle, argv[2]).c_str());
        for (mooon::sys::DBTable::size_type row=0; row<dbtable.size(); ++row)
        {
            const mooon::sys::DBRow& dbrow = dbtable[row];
            for (mooon::sys::DBTable::size_type col=0; col<dbrow.size(); ++col)
                printf("%s\b\n", dbrow[col].c_str());
            printf("\n");
        }
    }
    catch (mooon::sys::CDBException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }
#endif // MOOON_HAVE_MYSQL
    return 0;
}
