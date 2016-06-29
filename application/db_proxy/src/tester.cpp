// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "DbProxyService.h"
#include <iostream>
#include <mooon/net/thrift_helper.h>

static std::ostream& operator <<(std::ostream& out, const mooon::db_proxy::DBTable& db_table)
{
    for (size_t row=0; row<db_table.size(); ++row)
    {
        const mooon::db_proxy::DBRow& db_row = db_table[row];
        for (size_t col=0; col<db_row.size(); ++col)
        {
            out << db_row[col] << "\t";
        }
        out << std::endl;
    }

    return out;
}

int main()
{
    mooon::net::CThriftClientHelper<mooon::db_proxy::DbProxyServiceClient> _db_proxy("127.0.0.1", 8888);

    try
    {
        _db_proxy.connect();

        std::string result;
        std::vector<std::string> tokens;
        tokens.push_back("a");
        tokens.push_back("b");
        tokens.push_back("c");
        tokens.push_back("abc");

        std::string sign;
        mooon::db_proxy::DBTable db_table;
        _db_proxy->query(db_table, sign, 1, 1, tokens, 1, -1);
        std::cout << db_table << std::endl;

        db_table.clear();
        _db_proxy->query(db_table, sign, 1, 1, tokens, 2, 1);
        std::cout << db_table << std::endl;

        tokens.clear();
        tokens.push_back("abc");
        tokens.push_back("2015");
        tokens.push_back("year");
        tokens.push_back("11");
        int affected_rows = _db_proxy->update(sign, 1, 1, tokens);
        std::cout << "affected_rows: " << affected_rows << std::endl;
    }
    catch (apache::thrift::TApplicationException& tx)
    {
        printf("[1] %s\n", tx.what());
    }
    catch (std::exception& ex)
    {
        printf("[2] %s\n", ex.what());
    }

    return 0;
}
