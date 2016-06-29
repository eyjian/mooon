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
#include "mooon/net/listener.h"
#include "mooon/net/utils.h"
#include "mooon/net/tcp_waiter.h"
#include "mooon/sys/utils.h"
#include "mooon/utils/string_utils.h"
MOOON_NAMESPACE_USE

// 无参数时：
// 在0.0.0.0:5174上监听

// 一个参数时：
// 在0.0.0.0上监听
// argv[1]: 端口号

// 两个参数时：
// argv[1]: 监听的IP地址
// argv[2]: 监听的端口号
int main(int argc, char* argv[])
{
    uint16_t port;
    net::ip_address_t ip;
    net::CListener listener;

    if (1 == argc) // 无参数
    {
        ip = (char*)NULL;
        port = 5174; // 我(5)要(1)测(7)试(4)
    }
    else if (2 == argc) // 一个参数
    {
        ip = (char*)NULL;
        if (!utils::CStringUtils::string2uint16(argv[2], port))
        {
            fprintf(stderr, "Invalid port: %s.\n", argv[2]);
            exit(1);
        }
    }
    else if (3 == argc) // 两个参数
    {            
        ip =  argv[1];
        if (!utils::CStringUtils::string2uint16(argv[2], port))
        {
            fprintf(stderr, "Invalid port: %s.\n", argv[2]);
            exit(1);
        }
    }
    
    try
    {
        // 开始监听，允许在0.0.0.0上监听
        listener.listen(ip, port, true);
        fprintf(stdout, "Listening at %s:%d.\n", ip.to_string().c_str(), port);
        
        uint16_t peer_port;
        net::ip_address_t peer_ip;

        while (true)
        {
            // 接受一个连接请求
            int newfd = listener.accept(peer_ip, peer_port);
            fprintf(stdout, "Accepted connect - %s:%d.\n", peer_ip.to_string().c_str(), peer_port);

            // 将新的请求关联到CTcpWaiter上
            net::CTcpWaiter waiter;
            waiter.attach(newfd, peer_ip, peer_port);

            while (true)
            {
                // 接收数据
                char buffer[IO_BUFFER_MAX];
                ssize_t retval = waiter.receive(buffer, sizeof(buffer)-1);
                if (0 == retval)
                {           
                    // 对端关闭了连接
                    fprintf(stdout, "Connect closed by peer %s:%d.\n", peer_ip.to_string().c_str(), peer_port);
                    break;
                }
                else
                {
                    // 在屏幕上打印接收到的数据
                    buffer[retval] = '\0';
                    fprintf(stdout, "[R] ==> %s.\n", buffer);
                }
            }
        }
    }
    catch (sys::CSyscallException& ex)
    {
        // 监听或连接异常
        fprintf(stderr, "exception %s at %s:%d.\n", ex.str().c_str(), ex.file(), ex.line());
        exit(1);
    }

    return 0;
}
