// Writed by yijian (eyjian@qq.com, eyjian@gmail.com) on 2016/9/18
#include "DbProxyService.h"
#include <iostream>
#include <mooon/net/thrift_helper.h>

static std::ostream& operator <<(std::ostream& out, const mooon::db_proxy::DBTable& db_table)
{
    for (size_t row=0; row<db_table.size(); ++row)
    {
        const mooon::db_proxy::DBRow& db_row = db_table[row];
        for (size_t col=0; col<db_row.size(); ++col)
            out << db_row[col] << "\t";
        if (row < db_table.size()-1)
            out << std::endl;
    }

    return out;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "usage1: db_proxy_client db_proxy_ip db_proxy_port update_index token1 token2 ..." << std::endl
                  << "usage2: db_proxy_client db_proxy_ip db_proxy_port query_index limit_start limit token1 token2 ..." << std::endl;
        exit(1);
    }

    try
    {
        std::string db_proxy_ip = argv[1];
        uint16_t db_proxy_port = mooon::utils::CStringUtils::string2int<uint16_t>(argv[2]);
        mooon::net::CThriftClientHelper<mooon::db_proxy::DbProxyServiceClient> db_proxy(db_proxy_ip, db_proxy_port);
        db_proxy.connect();

        int i;
        int seq = 2016;
        int index = mooon::utils::CStringUtils::string2int<int>(argv[3]);
        std::string sign;
        std::vector<std::string> tokens;

        if (index >= 0)
        {
            for (i=4; i<argc; ++i)
                tokens.push_back(argv[i]);
            int affected_rows = db_proxy->update(sign, seq, index, tokens);
            std::cout << "affected_rows: " << affected_rows << std::endl;
        }
        else
        {
            mooon::db_proxy::DBTable dbtable;
            for (i=6; i<argc; ++i)
                tokens.push_back(argv[i]);
            int limit_start = mooon::utils::CStringUtils::string2int<int>(argv[4]);
            int limit = mooon::utils::CStringUtils::string2int<int>(argv[5]);
            db_proxy->query(dbtable, sign, seq, -index, tokens, limit, limit_start);
            if (dbtable.empty())
                std::cout << "empty" << std::endl;
            else
                std::cout << dbtable << std::endl;
        }
    }
    catch (apache::thrift::TApplicationException& tx)
    {
        std::cerr << "[1] " << tx.what() << std::endl;
        exit(1);
    }
    catch (std::exception& ex)
    {
        std::cerr << "[2] " << ex.what() << std::endl;
        exit(1);
    }
}
