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
 * Writed by yijian on 2015/7/29, eyjian@qq.com eyjian@gmail.com
 */
#ifndef MOOON_NET_LIBSSH2_H
#define MOOON_NET_LIBSSH2_H
#include "mooon/net/config.h"
#include "mooon/sys/syscall_exception.h"
#include <fstream>
#include <iostream>
NET_NAMESPACE_BEGIN

// 为非线程安全类
//
// 提供执行远程命令的能力，类似于ssh命令
// 可配合utils::CLoginTokener一起使用：#include <mooon/utils/tokener.h>
//
// 使用示例（执行远程命令）：
// try
// {
//     int exitcode;
//     std::string exitsignal;
//     std::string errmsg;
//     int num_bytes;
//     net::CLibssh2 libssh2(ip, port, username, password);
//     libssh2.remotely_execute(command, std::cout, &exitcode, &exitsignal, &errmsg, &num_bytes);
// }
// catch (sys::CSyscallException& syscall_ex)
// {
//     fprintf(stderr, "%s\n", syscall_ex.str().c_str());
// }
// catch (utils::CException& ex)
// {
//     fprintf(stderr, "%s\n", ex.str().c_str());
// }
class CLibssh2
{
public:
    // 初始化ssh2环境，为非线程安全函数，每个进程启动时调用一次
    static void init() throw (utils::CException);
    // 清理初始化时产生的资源，每个进程退出时调用一次，或者不再使用ssh2时调用一次
    static void fini();

public:
    // ip 远程主机sshd服务监听的IP地址
    // port 远程主机sshd服务监听的端口号
    // username 用来连接远程主机的用户名
    // password 用户名username的密码
    // timeout_seconds 连接超时时长，单位为秒
    // nonblocking 连接是否主国非阻塞方式，为true表示为非阻塞，为false表示为阻塞方式，建议采用非阻塞方式
    CLibssh2(const std::string& ip, uint16_t port, const std::string& username, const std::string& password, uint32_t timeout_seconds=2, bool nonblocking=true) throw (utils::CException, sys::CSyscallException);
    ~CLibssh2();
    int get_session_errcode() const;
    std::string get_session_errmsg() const;

    // command 被远程执行的命令，如：whoami
    // out 接收命令输出的流
    // exitcode 远程命令执行结束后的退出代码，如：0
    // exitsignal 远程命令执行时接收到的信号，如：TERM
    // num_bytes 远程命令吐出的字节数
    void remotely_execute(const std::string& command, std::ostream& out, int* exitcode, std::string* exitsignal, std::string* errmsg, int* num_bytes) throw (utils::CException, sys::CSyscallException);

private:
    void cleanup();
    void create_session(bool nonblocking);
    void set_known_hosts();
    void validate_authorization(const std::string& password);
    bool wait_socket();
    void handshake();
    void open_channel();
    int close_channel(std::string* exitsignal, std::string* errmsg);
    int read_channel(std::ostream& out);

private:
    int _socket_fd;
    void* _channel; // LIBSSH2_CHANNEL
    void* _session; // LIBSSH2_SESSION
    std::string _ip;
    uint16_t _port;
    std::string _username;
    uint32_t _timeout_seconds;
};

NET_NAMESPACE_END
#endif // MOOON_NET_LIBSSH2_H
