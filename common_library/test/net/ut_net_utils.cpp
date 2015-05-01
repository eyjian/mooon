/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include <mooon/net/utils.h>
#include <arpa/inet.h>
#include <stdio.h>
MOOON_NAMESPACE_USE

void test_host2net()
{
    //////////////////////////////////
    printf("\n\nTEST host2net\n");
    //////////////////////////////////
    
    // 转换2字节整数
    uint16_t b1;
    uint16_t a1 = 0x0103;

    net::CUtils::host2net<uint16_t>(a1, b1);
    printf("a1 = 0x%04x, b1=0x%04x\n", a1, b1);
    if (b1 == htons(a1)) /** 和系统库函数比较，以验证是否正确 */
        printf("host2net success\n");
    else
        printf("host2net failure\n");

    // 转换4字节整数
    printf("\n");
    uint32_t b2;
    uint32_t a2 = 0x01016070;

    net::CUtils::host2net<uint32_t>(a2, b2);
    printf("a2 = 0x%04x, b2=0x%04x\n", a2, b2);
    if (b2 == htonl(a2)) /** 和系统库函数比较，以验证是否正确 */
        printf("host2net success\n");
    else
        printf("host2net failure\n");

    // 按长度转换，应用到单字节字符串上，相当于反转字符串
    printf("\n");
    char str[] = "123456789";
    size_t length = strlen(str);
    char* dst = new char[length+1];
    utils::DeleteHelper<char> dh(dst, true); // 自动调用delete []dst
    net::CUtils::host2net(str, dst, length);
    dst[length] = '\0';
    printf("%s ==> %s\n",str, dst);
}

void test_get_ip_address()
{
    //////////////////////////////////
    printf("\n\nTEST test_get_ip_address\n");
    //////////////////////////////////

    std::string errinfo;
    net::string_ip_array_t ip_array;
    std::string hostname = "www.sina.com.cn";
    
    if (!net::CUtils::get_ip_address(hostname.c_str(), ip_array, errinfo))
    {
        fprintf(stderr, "get_ip_address error: %s.\n", errinfo.c_str());
    }
    else
    {
        for (net::string_ip_array_t::size_type i=0; i<ip_array.size(); ++i)
        {
            fprintf(stdout, "%s ===> %s\n", hostname.c_str(), ip_array[i].c_str());
        }
    }
}

int main()
{
    test_host2net();
    test_get_ip_address();
    return 0;
}
