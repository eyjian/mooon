#include <mooon/utils/tokener.h>
MOOON_NAMESPACE_USE

void print(const std::string& source, const std::string& sep, bool skip_sep=false)
{
    std::vector<std::string> tokens;
    int num_tokens = utils::CTokener::split(&tokens, source, sep, skip_sep);

    printf("source[%s]:\n", source.c_str());
    for (int i=0; i<num_tokens; ++i)
    {
        printf("token: \"%s\"\n", tokens[i].c_str());
    }

    printf("\n");
}

int main()
{
    std::string str1 = "abc##123##x#z##456";
    print(str1, "##");

    std::string str2 = "##abc##123##x#z##456";
    print(str2, "##");

    std::string str3 = "##abc##123##x#z##456##";
    print(str3, "##");

    std::string str4 = "###abc###123##x#z##456##";
    print(str4, "##");

    std::string str5 = "###abc###123##x#z###456##";
    print(str5, "###");

    std::string str6 = "##abc####123##x#z##456";
    print(str6, "##");

    std::string str7 = "##";
    print(str7, "##");

    std::string str8 = "";
    print(str8, "##");

    std::string str9 = "#abc#123#x#z#456#";
    print(str9, "#");

    std::string str0 = "##abc######123##x#z##456####";
    print(str0, "##");

    printf("//////////////////////////////////////\n");
    std::string strX1 = "abc  123    efg 456";
    std::string strX2 = "abc  123    efg 456 ";
    std::string strX3 = "abc  123    efg 456  ";
    std::string strX4 = " abc  123    efg 456  ";
    std::string strX5 = "  abc  123    efg 456  ";
    print(strX1, " ", true);
    print(strX2, " ", true);
    print(strX3, " ", true);
    print(strX4, " ", true);
    print(strX5, " ", true);

    printf("//////////////////////////////////////\n");
    std::string strZ1 = "abc\t\t123\t\t\t\tefg 456";
    std::string strZ2 = "abc\t\t123t\t\t\tefg 456\t";
    std::string strZ3 = "abc\t\t123\t\t\t\tefg\t456\t\t";
    std::string strZ4 = "\tabc\t\t123\t\t\tefg 456\t\t";
    std::string strZ5 = "\t\tabc\t\t123\t\t\t\tefg\t456\t\t";
    print(strZ1, "\t", true);
    print(strZ2, "\t", true);
    print(strZ3, "\t", true);
    print(strZ4, "\t", true);
    print(strZ5, "\t", true);

    return 0;
}
