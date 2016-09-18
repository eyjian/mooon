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
 * Author: jian yi, eyjian@qq.com
 */
#include "sys/signal_handler.h"
SYS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// 以下是CSignalHandler的实现
// ，将它放在示例代码之后，是为了方便看到如何使用CSignalHandler。

sigset_t CSignalHandler::_sigset;
std::vector<int> CSignalHandler::_signo_array;

bool CSignalHandler::ignore_signal(int signo) throw ()
{
    return signal(signo, SIG_IGN) != SIG_ERR;
}

bool CSignalHandler::block_signal(int signo) throw ()
{
    if (_signo_array.empty())
    {
        // 初始化_sigset
        if (-1 == sigemptyset(&_sigset))
        {
            return false;
        }
    }

    if (-1 == sigaddset(&_sigset, signo))
    {
        return false;
    }
    else
    {
        errno = pthread_sigmask(SIG_BLOCK, &_sigset, NULL);
        if (errno != 0)
        {
            return false;
        }
        else
        {
            _signo_array.push_back(signo);
            return true;
        }
    }
}

int CSignalHandler::wait_signal() throw ()
{
    int signo; // 发生的信号
    sigset_t sigset;

    // 初始化sigset
    if (-1 == sigemptyset(&sigset))
    {
        return -1;
    }

    // 设置sigset
    for (std::vector<int>::size_type i=0; i<_signo_array.size(); ++i)
    {
        if (-1 == sigaddset(&sigset, _signo_array[i]))
        {
            return -1;
        }
    }

    // 等待信号发生
    errno = sigwait(&sigset, &signo);
    if (errno != 0)
    {
        return -1;
    }

    return signo;
}

SYS_NAMESPACE_END
