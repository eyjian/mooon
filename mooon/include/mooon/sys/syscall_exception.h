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
#ifndef MOOON_SYS_SYSCALL_EXCEPTION_H
#define MOOON_SYS_SYSCALL_EXCEPTION_H
#include "mooon/sys/config.h"
#include "mooon/utils/exception.h"

#define THROW_SYSCALL_EXCEPTION(errmsg, errcode, syscall) \
    throw ::mooon::sys::CSyscallException(errmsg, errcode, __FILE__, __LINE__, syscall)

SYS_NAMESPACE_BEGIN

/** 系统调用出错异常，多数系统调用出错时，均以此异常型反馈给调用者 */
class CSyscallException: public utils::CException
{
public:
    CSyscallException(const char* errmsg, int errcode, const char* file, int line, const char* syscall) throw ();
    CSyscallException(const std::string& errmsg, int errcode, const std::string& file, int line, const std::string& syscall) throw ();
    virtual ~CSyscallException() throw () {}

    virtual std::string str() const throw ();
    const char* syscall() const throw ();

private:
    virtual std::string prefix() const throw ();

private:
    std::string _syscall; // 哪个系统调用失败了
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_SYSCALL_EXCEPTION_H
