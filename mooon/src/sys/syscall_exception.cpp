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
#include <sstream>
#include <string.h>
#include "sys/utils.h"
#include "sys/syscall_exception.h"
SYS_NAMESPACE_BEGIN

CSyscallException::CSyscallException(
        const char* errmsg, int errcode, const char* file, int line, const char* syscall) throw ()
    : utils::CException(errmsg, errcode, file, line)
{
    if (NULL == errmsg)
        _errmsg = strerror(errno);

    if (syscall != NULL)
        _syscall = syscall;
}

CSyscallException::CSyscallException(
        const std::string& errmsg, int errcode, const std::string& file, int line, const std::string& syscall) throw ()
    : CException(errmsg, errcode, file, line)
{
    if (errmsg.empty())
        _errmsg = strerror(errno);

    if (!syscall.empty())
        _syscall = syscall;
}

std::string CSyscallException::str() const throw ()
{
    std::stringstream ss;

    ss << prefix() << _errmsg << "@" << _file << ":" << _line;
    if (!_syscall.empty())
        ss << "#" << _syscall;

    return ss.str();
}

const char* CSyscallException::syscall() const throw ()
{
    return _syscall.c_str();
}

std::string CSyscallException::prefix() const throw ()
{
    return std::string("syscall_exception://");
}

SYS_NAMESPACE_END
