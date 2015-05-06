#include <mooon/utils/tokener.h>
MOOON_NAMESPACE_USE

void print(const std::string& source, const std::string& sep)
{
    std::vector<std::string> tokens;
    int num_tokens = utils::CTokener::split(&tokens, source, sep);

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

    return 0;
}
