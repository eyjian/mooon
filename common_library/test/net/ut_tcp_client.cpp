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
#include "mooon/net/utils.h"
#include "mooon/sys/utils.h"
#include "mooon/net/tcp_client.h"
#include "mooon/utils/string_utils.h"
MOOON_NAMESPACE_USE

// 需要两个参数：
// argv[1]: 连接IP地址
// argv[2]: 连接的端口号
int main(int argc, char* argv[])
{    
    uint16_t port;
    net::ip_address_t ip;
    net::CTcpClient client;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s ip port\n"
            , sys::CUtils::get_program_short_name());
        exit(1);
    }
             
    ip =  argv[1];
    if (!utils::CStringUtils::string2uint16(argv[2], port))
    {
        fprintf(stderr, "Invalid port: %s.\n", argv[2]);
        exit(1);
    }
    
    try
    {
        // 设置IP和端口
        client.set_peer_ip(ip);
        client.set_peer_port(port);

        // 执行连接
        client.timed_connect();
        fprintf(stdout, "Connected %s:%d success.\n", ip.to_string().c_str(), port);

        while (true)
        {
            // 从标准输出读入数据，并发送给远端
            char line[LINE_MAX];
            fgets(line, sizeof(line)-1, stdin);
            
            client.send(line, strlen(line));
        }
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接异常退出
        fprintf(stderr, "exception %s at %s:%d.\n", ex.str().c_str(), ex.file(), ex.line());
        exit(1);
    }

    return 0;
}
