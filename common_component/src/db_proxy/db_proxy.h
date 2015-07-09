// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_H
#define MOOON_DB_PROXY_H
#include <ostream>
#include "rpc/db_proxy_types.h"

std::ostream& operator <<(std::ostream& out, const mooon::db_proxy::DBTable& db_table)
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

namespace mooon { namespace db_proxy {

} // namespace mooon
} // namespace db_proxy
#endif // MOOON_DB_PROXY_H
