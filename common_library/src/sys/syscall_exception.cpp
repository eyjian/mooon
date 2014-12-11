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
#include "sys/util.h"
#include "sys/syscall_exception.h"
SYS_NAMESPACE_BEGIN

CSyscallException::CSyscallException(int errcode, const char* filename, int linenumber, const char* tips)
    :_errcode(errcode)
    ,_linenumber(linenumber)
{
    strncpy(_filename, filename, sizeof(_filename)-1);
    _filename[sizeof(_filename)-1] = '\0';
    if (tips != NULL) _tips = tips;
}

std::string CSyscallException::get_errmessage() const
{
    return CUtil::get_error_message(get_errcode());
}

std::string CSyscallException::to_string() const
{
    std::stringstream ss;
    ss << "syscall_exception://"
       << get_errcode() << ":"
       << get_errmessage() << "@"
       << get_linenumber() << ":"
       << get_filename();

    return ss.str();
}

SYS_NAMESPACE_END
