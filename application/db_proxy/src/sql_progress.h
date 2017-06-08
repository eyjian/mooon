// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_DB_PROXY_SQL_PROGRESS_H
#define MOOON_DB_PROXY_SQL_PROGRESS_H
#include <mooon/utils/string_utils.h>
#include <zlib.h>
namespace mooon { namespace db_proxy {

#pragma pack(4)
struct Progress
{
    uint32_t crc32;
    uint32_t offset;
    char filename[sizeof("sql.0001496887700.000001")]; // 包含结尾符'\0'

    Progress()
    {
        memset(this, sizeof(*this), 0);
    }

    bool empty() const
    {
        return '\0' == filename[0];
    }

    std::string str() const
    {
        return utils::CStringUtils::format_string("progress://C%u/O%u/F%s", crc32, offset, filename);
    }

    uint32_t get_crc32() const
    {
        const std::string crc32_str = utils::CStringUtils::format_string("%u%s", offset, filename);
        return ::crc32(0L, (const unsigned char*)crc32_str.data(), crc32_str.size());
    }
};
#pragma pack()

}} // namespace mooon { namespace db_proxy {
#endif // MOOON_DB_PROXY_SQL_PROGRESS_H
