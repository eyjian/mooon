#include "mooon/utils/string_utils.h"
UTILS_NAMESPACE_USE

int main()
{
    // 测试string2int32函数
    printf("\n>>>>>>>>>>TEST string2int32<<<<<<<<<<\n\n");
    
    // 测试1
    int result1;
    const char* str1 = "123456";    
    if (CStringUtils::string2int32(str1, result1))
        printf("%s ===> %d\n", str1, result1);
    else
        printf("ERROR string2int32: %s\n", str1);

    // 测试2
    int result2;
    const char* str2 = "123a456";    
    if (CStringUtils::string2int32(str2, result2))
        printf("%s ===> %d\n", str2, result2);
    else
        printf("ERROR string2int32: %s\n", str2);

    // 测试3
    int result3;
    const char* str3 = "123a456";    
    if (CStringUtils::string2int32(str3, result3, 3))
        printf("%s ===> %d\n", str3, result3);
    else
        printf("ERROR string2int32: %s\n", str3);


    // 测试4
    int result4;
    const char* str4 = "-123456";    
    if (CStringUtils::string2int32(str4, result4))
        printf("%s ===> %d\n", str4, result4);
    else
        printf("ERROR string2int32: %s\n", str4);

    // 测试5
    int result5;
    const char* str5 = "0123456";    
    if (CStringUtils::string2int32(str5, result5))
        printf("%s ===> %d\n", str5, result5);
    else
        printf("ERROR string2int32: %s\n", str5);

    // 测试6
    int result6;
    const char* str6 = "-0";    
    if (CStringUtils::string2int32(str6, result6))
        printf("%s ===> %d\n", str6, result6);
    else
        printf("ERROR string2int32: %s\n", str6);

    // 测试7
    int result7;
    const char* str7 = "-023";    
    if (CStringUtils::string2int32(str7, result7))
        printf("%s ===> %d\n", str7, result7);
    else
        printf("ERROR string2int32: %s\n", str7);

    printf("\n\n>>>>>>>>>>TEST trim/trim_left/trim_right<<<<<<<<<<\n\n");

    // 测试8
    char str8[] = " def";
    printf("abc%s\n", str8);
    CStringUtils::trim_left(str8);
    printf("abc%s\n", str8);

    // 测试9
    char str9[] = "abc  ";
    printf("%sdef\n", str9);
    CStringUtils::trim_right(str9);
    printf("%sdef\n", str9);

    // 测试10
    char str10[] = " 456 ";
    printf("123%s789\n", str10);
    CStringUtils::trim(str10);
    printf("123%s789\n", str10);

    return 0;    
}
