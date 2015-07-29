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

class CLibssh2
{
public:
    static void init() throw (utils::CException);
    static void fini();

public:
    CLibssh2(const std::string& ip, uint16_t port, const std::string& username, const std::string& password, uint32_t timeout_seconds=2, bool nonblocking=true) throw (utils::CException, sys::CSyscallException);
    ~CLibssh2();
    int get_session_errcode() const;
    std::string get_session_errmsg() const;

    void remotely_execute(const std::string& command, std::ostream& out, int* exitcode, std::string* exitsignal, std::string* errmsg, int* num_bytes) throw (utils::CException, sys::CSyscallException);

private:
    bool wait_socket();

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
