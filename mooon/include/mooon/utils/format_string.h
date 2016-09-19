// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#ifndef MOOON_UTILS_FORMAT_STRING_H
#define MOOON_UTILS_FORMAT_STRING_H
#include <mooon/utils/string_utils.h>
#include <string>
#include <vector>
UTILS_NAMESPACE_BEGIN

enum { FORMAT_STRING_SIZE = 56 };
inline std::string format_string(const char* format, const std::vector<std::string>& tokens)
{
    // 确保"%s"的个数为tokens.size()
    std::vector<std::string>::size_type excepted_num_tokens = 0;
    const char* format_p = format;
    while (*format_p != '\0')
    {
        if (*format_p != '%')
        {
            ++format_p;
        }
        else
        {
            ++format_p;
            if (('\0' == *format_p) || (*format_p != 's'))
                return std::string(""); // error，只能为%s，不能为其它如%d等，也不能为单个的%

            ++excepted_num_tokens;
            ++format_p;
        }
    }
    if (excepted_num_tokens != tokens.size())
    {
        return std::string("");
    }

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
    if (30 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str());
    }
    if (31 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str());
    }
    if (32 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str());
    }
    if (33 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str());
    }
    if (34 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str());
    }
    if (35 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str());
    }
    if (36 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str());
    }
    if (37 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str());
    }
    if (38 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str());
    }
    if (39 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str());
    }
    if (40 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str());
    }
    if (41 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str());
    }
    if (42 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str());
    }
    if (43 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str());
    }
    if (44 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str());
    }
    if (45 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str());
    }
    if (46 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str());
    }
    if (47 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str());
    }
    if (48 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str());
    }
    if (49 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str());
    }
    if (50 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str());
    }
    if (51 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str(), tokens[50].c_str());
    }
    if (52 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str(), tokens[50].c_str(), tokens[51].c_str());
    }
    if (53 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str(), tokens[50].c_str(), tokens[51].c_str(), tokens[52].c_str());
    }
    if (54 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str(), tokens[50].c_str(), tokens[51].c_str(), tokens[52].c_str(), tokens[53].c_str());
    }
    if (55 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str(), tokens[50].c_str(), tokens[51].c_str(), tokens[52].c_str(), tokens[53].c_str(), tokens[54].c_str());
    }
    if (56 == tokens.size())
    {
        return CStringUtils::format_string(format, tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str(), tokens[4].c_str(), tokens[5].c_str(), tokens[6].c_str(), tokens[7].c_str(), tokens[8].c_str(), tokens[9].c_str(), tokens[10].c_str(), tokens[11].c_str(), tokens[12].c_str(), tokens[13].c_str(), tokens[14].c_str(), tokens[15].c_str(), tokens[16].c_str(), tokens[17].c_str(), tokens[18].c_str(), tokens[19].c_str(), tokens[20].c_str(), tokens[21].c_str(), tokens[22].c_str(), tokens[23].c_str(), tokens[24].c_str(), tokens[25].c_str(), tokens[26].c_str(), tokens[27].c_str(), tokens[28].c_str(), tokens[29].c_str(), tokens[30].c_str(), tokens[31].c_str(), tokens[32].c_str(), tokens[33].c_str(), tokens[34].c_str(), tokens[35].c_str(), tokens[36].c_str(), tokens[37].c_str(), tokens[38].c_str(), tokens[39].c_str(), tokens[40].c_str(), tokens[41].c_str(), tokens[42].c_str(), tokens[43].c_str(), tokens[44].c_str(), tokens[45].c_str(), tokens[46].c_str(), tokens[47].c_str(), tokens[48].c_str(), tokens[49].c_str(), tokens[50].c_str(), tokens[51].c_str(), tokens[52].c_str(), tokens[53].c_str(), tokens[54].c_str(), tokens[55].c_str());
    }

    //MOOON_ASSERT(false);
    return std::string("");
}

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_FORMAT_STRING_H

