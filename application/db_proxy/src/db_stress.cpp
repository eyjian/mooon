// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/scoped_ptr.h>

// DB IP
STRING_ARG_DEFINE(db_ip, "127.0.0.1", "listen IP address of database");
// DB端口
INTEGER_ARG_DEFINE(uint16_t, db_port, 3306, 1000, 65535, "listen port of database");
// DB username
STRING_ARG_DEFINE(db_user, "root", "database username");
// DB password
STRING_ARG_DEFINE(db_passwd, "", "database password");
// DB name
STRING_ARG_DEFINE(db_name, "test", "database name");

// 压测线程数
INTEGER_ARG_DEFINE(uint16_t, threads, 1, 1, 100, "number of threads to stress");
// 压力测试用的SQL
STRING_ARG_DEFINE(sql, "", "stress sql");

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return false;
    }

    return 0;
}
