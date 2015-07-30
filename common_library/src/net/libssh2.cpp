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
#include "net/libssh2.h"
#include "mooon/net/config.h"
#include <arpa/inet.h>
#include <mooon/net/utils.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#if HAVE_LIBSSH2 == 1
#include <libssh2.h>
#endif // HAVE_LIBSSH2

NET_NAMESPACE_BEGIN
#if HAVE_LIBSSH2 == 1

void CLibssh2::init() throw (utils::CException)
{
    int errcode = libssh2_init(0);
    if (errcode != 0)
        THROW_EXCEPTION("init libssh2 failed", errcode);
}

void CLibssh2::fini()
{
    libssh2_exit();
}

CLibssh2::CLibssh2(const std::string& ip, uint16_t port, const std::string& username, const std::string& password, uint32_t timeout_seconds, bool nonblocking) throw (utils::CException, sys::CSyscallException)
    : _socket_fd(-1), _channel(NULL), _session(NULL), _ip(ip), _port(port), _username(username), _timeout_seconds(timeout_seconds)
{
    create_session(nonblocking);

    try
    {
        handshake();
        set_known_hosts();
        validate_authorization(password);
    }
    catch (...)
    {
        cleanup();
        throw;
    }
}

CLibssh2::~CLibssh2()
{
    cleanup();
}

int CLibssh2::get_session_errcode() const
{
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);
    return libssh2_session_last_errno(session);
}

std::string CLibssh2::get_session_errmsg() const
{
    std::string result;
    char* errmsg;
    int errmsg_len = 0;
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);

    (void)libssh2_session_last_error(session, &errmsg, &errmsg_len, 1);
    if (errmsg != NULL)
    {
        result = errmsg;
        free(errmsg);
    }

    return result;
}

void CLibssh2::remotely_execute(
    const std::string& command, std::ostream& out,
    int* exitcode, std::string* exitsignal, std::string* errmsg, int* num_bytes) throw (utils::CException, sys::CSyscallException)
{
    open_channel();

    try
    {
        while (true)
        {
            LIBSSH2_CHANNEL* channel = static_cast<LIBSSH2_CHANNEL*>(_channel);
            int errcode = libssh2_channel_exec(channel, command.c_str());
            if (0 == errcode)
            {
                break;
            }
            else if (errcode != LIBSSH2_ERROR_EAGAIN)
            {
                THROW_EXCEPTION(get_session_errmsg(), get_session_errcode());
            }
        }

        *num_bytes = read_channel(out);
    }
    catch (...)
    {
        *exitcode = close_channel(exitsignal, errmsg);
        throw;
    }

    // 不能放在try{}中，
    // 否则可能会触发递归调用close_channel()，原因是close_channel()也抛异常
    *exitcode = close_channel(exitsignal, errmsg);
}

void CLibssh2::cleanup()
{
    if (_channel != NULL)
    {
        LIBSSH2_CHANNEL* channel = static_cast<LIBSSH2_CHANNEL*>(_channel);
        libssh2_channel_free(channel);
        _channel = NULL;
    }
    if (_session != NULL)
    {
        LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);
        libssh2_session_disconnect(session, "normal Shutdown");
        libssh2_session_free(session);
        _session = NULL;
    }
    if (_socket_fd != -1)
    {
        close(_socket_fd);
        _socket_fd = -1;
    }
}

void CLibssh2::create_session(bool nonblocking)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == socket_fd)
    {
        THROW_SYSCALL_EXCEPTION(strerror(errno), errno, "socket");
    }

    try
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);
        addr.sin_addr.s_addr = inet_addr(_ip.c_str());

        if (-1 == connect(socket_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)))
        {
            THROW_SYSCALL_EXCEPTION(strerror(errno), errno, "connect");
        }

        // 创建ssh2会话
        LIBSSH2_SESSION* session = libssh2_session_init();
        if (nonblocking) // 设置为非阻塞
            libssh2_session_set_blocking(session, 0);

        _session = session;
        _socket_fd = socket_fd;
    }
    catch (...)
    {
        close(socket_fd);
    }
}

void CLibssh2::set_known_hosts()
{
    int type;
    int check;
    size_t len;
    struct libssh2_knownhost* known_host;
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);
    LIBSSH2_KNOWNHOSTS* known_hosts = libssh2_knownhost_init(session);

    if (NULL == known_hosts)
    {
        THROW_EXCEPTION(get_session_errmsg(), get_session_errcode());
    }

    try
    {
        // read all hosts from here
        libssh2_knownhost_readfile(known_hosts, "known_hosts", LIBSSH2_KNOWNHOST_FILE_OPENSSH);
        // store all known hosts to here
        libssh2_knownhost_writefile(known_hosts, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

        const char* fingerprint = libssh2_session_hostkey(session, &len, &type);
        if (NULL == fingerprint)
        {
            THROW_EXCEPTION(get_session_errmsg(), get_session_errcode());
        }

    #if LIBSSH2_VERSION_NUM >= 0x010206
        // introduced in 1.2.6
        check = libssh2_knownhost_checkp(
            known_hosts, _ip.c_str(), _port,
            fingerprint, len, LIBSSH2_KNOWNHOST_TYPE_PLAIN|LIBSSH2_KNOWNHOST_KEYENC_RAW, &known_host);
    #else
        // 1.2.5 or older
        check = libssh2_knownhost_check(
            known_hosts, _ip.c_str(),
            fingerprint, len, LIBSSH2_KNOWNHOST_TYPE_PLAIN|LIBSSH2_KNOWNHOST_KEYENC_RAW, &known_host);
    #endif // LIBSSH2_VERSION_NUM

        libssh2_knownhost_free(known_hosts);
    }
    catch (...)
    {
        libssh2_knownhost_free(known_hosts);
        throw;
    }
}

void CLibssh2::validate_authorization(const std::string& password)
{
    int errcode;
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);

    while (true)
    {
        if (!password.empty())
        {
            errcode = libssh2_userauth_password(session, _username.c_str(), password.c_str());
        }
        else
        {
            errcode = libssh2_userauth_publickey_fromfile(session, _username.c_str(),
                "/home/user/.ssh/id_rsa.pub", "/home/user/.ssh/id_rsa", password.c_str());
        }

        if (0 == errcode)
        {
            break;
        }
        else if (errcode != LIBSSH2_ERROR_EAGAIN)
        {
            THROW_EXCEPTION(get_session_errmsg(), get_session_errcode());
        }
    }
}

// http://www.libssh2.org/examples/ssh2_exec.html
bool CLibssh2::wait_socket()
{
    int events_requested = 0;
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);

    // now make sure we wait in the correct direction
    int direction = libssh2_session_block_directions(session);
    if(direction & LIBSSH2_SESSION_BLOCK_INBOUND)
        events_requested |= POLLIN;
    if(direction & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        events_requested |= POLLOUT;

    return CUtils::timed_poll(_socket_fd, events_requested, _timeout_seconds*1000);
}

void CLibssh2::handshake()
{
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);

    while (true)
    {
        // ssh2握手
        int errcode = libssh2_session_handshake(session, _socket_fd);
        if (0 == errcode)
        {
            break;
        }
        else if (errcode != LIBSSH2_ERROR_EAGAIN)
        {
            THROW_EXCEPTION(get_session_errmsg(), get_session_errcode());
        }
        else if (!wait_socket())
        {
            THROW_SYSCALL_EXCEPTION("handshake timeout", ETIMEDOUT, "poll");
        }
    }
}

void CLibssh2::open_channel()
{
    LIBSSH2_CHANNEL* channel = NULL;
    LIBSSH2_SESSION* session = static_cast<LIBSSH2_SESSION*>(_session);

    // 建立会话通道
    while (true)
    {
        channel = libssh2_channel_open_session(session);
        if (channel != NULL)
            break;

        int errcode = get_session_errcode();
        if (errcode != LIBSSH2_ERROR_EAGAIN)
        {
            THROW_EXCEPTION(get_session_errmsg(), errcode);
        }
        else if (!wait_socket())
        {
            THROW_SYSCALL_EXCEPTION("open session timeout", ETIMEDOUT, "poll");
        }
    }

    _channel = channel;
}

int CLibssh2::close_channel(std::string* exitsignal, std::string* errmsg)
{
    int exitcode = 127; // 127: command not exists
    LIBSSH2_CHANNEL* channel = static_cast<LIBSSH2_CHANNEL*>(_channel);

    while (_channel != NULL)
    {
        int errcode = libssh2_channel_close(channel);
        if (0 == errcode)
        {
            char* errmsg_ = NULL;
            char* exitsignal_ = NULL;

            exitcode = libssh2_channel_get_exit_status(channel);
            if (exitcode != 0) // 0 success
            {
                // 调用端可以strerror(*exitcode)取得出错原因
                libssh2_channel_get_exit_signal(channel, &exitsignal_, NULL, &errmsg_, NULL, NULL, NULL);
                if (errmsg_ != NULL)
                {
                    *errmsg = errmsg_;
                    free(errmsg_);
                }
                if (exitsignal_ != NULL)
                {
                    *exitsignal = exitsignal_;
                    free(exitsignal_);
                }
            }

            _channel = NULL;
            break;
        }
        else if (LIBSSH2_ERROR_EAGAIN == errcode)
        {
            if (!wait_socket())
            {
                THROW_SYSCALL_EXCEPTION("channel close timeout", ETIMEDOUT, "poll");
            }
        }
    }

    return exitcode;
}

int CLibssh2::read_channel(std::ostream& out)
{
    int num_bytes = 0;
    LIBSSH2_CHANNEL* channel = static_cast<LIBSSH2_CHANNEL*>(_channel);

    while (true)
    {
        char buffer[4096];
        int bytes = libssh2_channel_read(channel, buffer, sizeof(buffer)-1);

        if (0 == bytes)
        {
            break; // connection closed now
        }
        else if (bytes > 0)
        {
            num_bytes += bytes;
            buffer[bytes] = '\0';
            out << buffer;
        }
        else
        {
            int errcode = get_session_errcode();

            if (errcode != LIBSSH2_ERROR_EAGAIN)
            {
                THROW_EXCEPTION(get_session_errmsg(), errcode);
            }
            else if (!wait_socket())
            {
                THROW_SYSCALL_EXCEPTION("channel read timeout", ETIMEDOUT, "poll");
            }
        }
    }
}

#endif // HAVE_LIBSSH2
NET_NAMESPACE_END
