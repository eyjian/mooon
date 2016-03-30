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

// Linux下编译libssh2，需要指定参数“--with-libssl-prefix”：
// ./configure --prefix=/usr/local/libssh2-1.6.0 --with-libssl-prefix=/usr/local/openssl
// 因此，编译之前需要先安装好openssl
//
// 基于libssh2（http://www.libssh2.org/）的实现，
// 请使用时尊重libssh2的版本
// 以下为libssh2的版本说明：
/* Copyright (c) 2004-2009, Sara Golemon <sarag@libssh2.org>
 * Copyright (c) 2009-2015 Daniel Stenberg
 * Copyright (c) 2010 Simon Josefsson <simon@josefsson.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *   Redistributions of source code must retain the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials
 *   provided with the distribution.
 *
 *   Neither the name of the copyright holder nor the names
 *   of any other contributors may be used to endorse or
 *   promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
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
    CLibssh2(const std::string& ip, uint16_t port, const std::string& username, const std::string& password, uint32_t timeout_seconds=2, bool nonblocking=false) throw (utils::CException, sys::CSyscallException);
    ~CLibssh2();

    // command 被远程执行的命令，如：whoami
    // out 接收命令输出的流
    // exitcode 远程命令执行结束后的退出代码，如：0
    // exitsignal 远程命令执行时接收到的信号，如：TERM
    // num_bytes 远程命令吐出的字节数
    void remotely_execute(const std::string& command, std::ostream& out, int* exitcode, std::string* exitsignal, std::string* errmsg, int* num_bytes) throw (utils::CException, sys::CSyscallException);

    // 下载远端的文件到本地
    // remote_filepath 被下载的远端文件
    // num_bytes 远端文件的字节数
    void download(const std::string& remote_filepath, std::ostream& out, int* num_bytes) throw (utils::CException, sys::CSyscallException);

    // 上传本地文件到远端
    // num_bytes 本地文件的字节数
    void upload(const std::string& local_filepath, const std::string& remote_filepath, int* num_bytes) throw (utils::CException, sys::CSyscallException);

private:
    int get_session_errcode() const;
    std::string get_session_errmsg() const;
    void cleanup();
    void create_session(bool nonblocking);
    void set_known_hosts();
    void validate_authorization(const std::string& password);
    bool timedwait_socket();
    void handshake();
    void* open_ssh_channel();
    void* open_scp_read_channel(const std::string& remote_filepath);

    // mtime 最近一次修改时间
    // atime 最近一次访问时间
    void* open_scp_write_channel(const std::string& remote_filepath, int filemode, size_t filesize, time_t mtime, time_t atime);
    int close_ssh_channel(void* channel, std::string* exitsignal, std::string* errmsg);
    int read_channel(void* channel, std::ostream& out);
    void write_channel(void* channel, const char *buffer, size_t buffer_size);

private:
    int _socket_fd;
    void* _session; // LIBSSH2_SESSION
    std::string _ip;
    uint16_t _port;
    std::string _username;
    uint32_t _timeout_seconds;
};

NET_NAMESPACE_END
#endif // MOOON_NET_LIBSSH2_H
