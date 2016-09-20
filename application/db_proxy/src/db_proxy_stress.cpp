// Writed by yijian (eyjian@qq.com, eyjian@gmail.com) on 2016/9/18
// benchmark tool
#include "config_loader.h"
#include "DbProxyService.h"
#include <iostream>
#include <mooon/net/thrift_helper.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/tokener.h>

STRING_ARG_DEFINE(ip, "127.0.0.1", "db proxy IP address");
INTEGER_ARG_DEFINE(uint16_t, port, 4077, 1000, 5000, "db proxy listen port");
INTEGER_ARG_DEFINE(uint8_t, threads, 1, 1, 100, "number of threads to test");
INTEGER_ARG_DEFINE(uint32_t, number, 1, 1, std::numeric_limits<uint32_t>::max(), "number of requests of every stress thread");
INTEGER_ARG_DEFINE(int16_t, index, 0, 0, mooon::db_proxy::MAX_SQL_TEMPLATE, "query or update index");
STRING_ARG_DEFINE(tokens, "", "tokens separated by comma");

static void stress_thread();

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        std::cerr << errmsg << std::endl
                  << mooon::utils::g_help_string << std::endl;
        exit(1);
    }
    if (mooon::argument::tokens->value().empty())
    {
        std::cerr << "parameter[--tokens] is empty" << std::endl
                  << mooon::utils::g_help_string << std::endl;
        exit(1);
    }

    mooon::sys::CThreadEngine** stress_threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        stress_threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(stress_thread));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        stress_threads[i]->join();
        delete stress_threads[i];
    }
    delete []stress_threads;
    return 0;
}

void stress_thread()
{
    std::string db_proxy_ip = mooon::argument::ip->value();
    uint16_t db_proxy_port = mooon::argument::port->value();
    mooon::net::CThriftClientHelper<mooon::db_proxy::DbProxyServiceClient> db_proxy(db_proxy_ip, db_proxy_port);
    db_proxy.connect();

    int limit_start = 0;
    int limit = 10;
    int index = mooon::argument::index->value();
    std::string sign;
    std::vector<std::string> tokens;

    mooon::utils::CTokener::split(&tokens, mooon::argument::tokens->value(), ",");
    for (uint32_t i=0; i<mooon::argument::number->value(); ++i)
    {
        try
        {
            int seq = static_cast<int>(i);
            if (index > 0)
            {
                db_proxy->update(sign, seq, index, tokens);
            }
            else
            {
                mooon::db_proxy::DBTable dbtable;
                db_proxy->query(dbtable, sign, seq, -index, tokens, limit, limit_start);
            }
        }
        catch (apache::thrift::transport::TTransportException& ex)
        {
            std::cerr << "TransportException: " << ex.what() << std::endl;
            db_proxy.close();
            db_proxy.connect();
        }
        catch (apache::thrift::TApplicationException& ex)
        {
            std::cerr << "ApplicationException: " << ex.what() << std::endl;
        }
    }
}
