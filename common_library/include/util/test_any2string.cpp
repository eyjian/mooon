// Write by yijian on 2014/12/27
// used to test any2string.h
#include "any2string.h"
#include <stdio.h>

int main()
{
    printf("%s\n", any2string(1).c_str());
    printf("%s\n", any2string(1, "2").c_str());
    printf("%s\n", any2string(1, "2", 3).c_str());
    printf("%s\n", any2string(1, "2", 3, "4").c_str());
    printf("%s\n", any2string(1, '-', "2", "-", 3, "-", "4", '-', "5").c_str());
    printf("%s\n", any2string("https", ':', "//", "github", '.', std::string("com"), '/', "eyjian", '/', std::string("mooon")).c_str());

    return 0;
}

