// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_UTILS_FORMAT_STRING_H
#define MOOON_UTILS_FORMAT_STRING_H
#include <mooon/utils/string_utils.h>
#include <string>
#include <vector>
UTILS_NAMESPACE_BEGIN

inline std::string format_string(const char* format, const std::vector<std::string>& tokens)
{
    if (0 == tokens.size())
    {
        return CStringUtils::format_string(format);
    }
    if (1 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str());
    }
    if (2 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str());
    }
    if (3 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str());
    }
    if (4 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str());
    }
    if (5 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str());
    }
    if (6 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str());
    }
    if (7 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str());
    }
    if (8 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str());
    }
    if (9 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str());
    }
    if (10 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str());
    }
    if (11 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str());
    }
    if (12 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str());
    }
    if (13 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str());
    }
    if (14 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str());
    }
    if (15 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str());
    }
    if (16 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str());
    }
    if (17 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str());
    }
    if (18 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str());
    }
    if (19 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str());
    }
    if (20 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str());
    }
    if (21 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str());
    }
    if (22 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str());
    }
    if (23 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str());
    }
    if (24 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str());
    }
    if (25 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str());
    }
    if (26 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str());
    }
    if (27 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str());
    }
    if (28 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str());
    }
    if (29 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str());
    }
}

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_FORMAT_STRING_H

