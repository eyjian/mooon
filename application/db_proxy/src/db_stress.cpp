// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/stop_watch.h>
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

// 压测的SQL数
INTEGER_ARG_DEFINE(uint32_t, num, 1, 1, std::numeric_limits<uint32_t>::max(), "number of sql");
// 压测线程数
INTEGER_ARG_DEFINE(uint8_t, threads, 1, 1, 100, "number of threads to stress");
// 压力测试用的SQL
STRING_ARG_DEFINE(sql, "", "stress sql");

// 批量提交的SQL数
INTEGER_ARG_DEFINE(uint8_t, batch, 1, 1, std::numeric_limits<uint8_t>::max(), "number of sql to batch to commit");

static atomic_t sg_count = 0;
static void stress_thread(uint8_t index);

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return 1;
    }
    if (!mooon::sys::CMySQLConnection::library_init())
    {
        fprintf(stderr, "init mysql failed\n");
        return 1;
    }

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** stress_threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        stress_threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(stress_thread, i));
    }
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        stress_threads[i]->join();
        delete stress_threads[i];
    }

    int count = atomic_read(&sg_count);
    uint32_t microseconds = stop_watch.get_elapsed_microseconds();
    uint32_t milliseconds = microseconds / 1000;
    uint32_t seconds = microseconds / 1000000;
    fprintf(stdout, "count: %d\n", count);
    fprintf(stdout, "milliseconds: %u, seconds: %u\n", milliseconds, seconds);
    if (0 == seconds)
        fprintf(stdout, "qps: %d\n", count);
    else
        fprintf(stdout, "qps: %d, %d\n", count/seconds, count/seconds/mooon::argument::threads->value());

    delete []stress_threads;
    mooon::sys::CMySQLConnection::library_end();
    return 0;
}

void stress_thread(uint8_t index)
{
    mooon::sys::CMySQLConnection mysql;
    mysql.set_host(mooon::argument::db_ip->value(), mooon::argument::db_port->value());
    mysql.set_user(mooon::argument::db_user->value(), mooon::argument::db_passwd->value());
    mysql.set_db_name(mooon::argument::db_name->value());

    try
    {
        mysql.open();
        if (mooon::argument::batch->value() > 1)
            mysql.enable_autocommit(false);

        for (uint32_t i=0; i<mooon::argument::num->value();)
        {
            for (uint8_t j=0; j<mooon::argument::batch->value()&&i<mooon::argument::num->value(); ++j,++i)
            {
                mysql.update("%s", mooon::argument::sql->c_value());
                atomic_inc(&sg_count);
            }

            if (mooon::argument::batch->value() > 1)
                mysql.commit();
        }
    }
    catch (mooon::sys::CDBException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }
}
