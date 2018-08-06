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
#include "utils/exception.h"
#include "utils/string_utils.h"
#include <sstream>
UTILS_NAMESPACE_BEGIN

CException::CException(const char* errmsg, int errcode, const char* file, int line) throw ()
{
    init(errmsg, errcode, file, line);
}

CException::CException(const std::string& errmsg, int errcode, const std::string& file, int line) throw ()
{
    init(errmsg.c_str(), errcode, file.c_str(), line);
}

const char* CException::what() const throw ()
{
    return _errmsg.c_str();
}

const char* CException::file() const throw ()
{
    return _file.c_str();
}

int CException::line() const throw ()
{
    return _line;
}

int CException::errcode() const throw ()
{
    return _errcode;
}

std::string CException::str() const throw ()
{
    std::stringstream ss;
    ss << prefix() << "[" << _errcode << "]" << _errmsg << "@" << _file << ":" << _line;

    return ss.str();
}

std::string CException::prefix() const throw ()
{
    return "exception://";
}

void CException::init(const char* errmsg, int errcode, const char* file, int line) throw ()
{
    if (errmsg != NULL)
        _errmsg = errmsg;
    _errcode = errcode;

    _line = line;
    if (file != NULL)
    {
        _file = utils::CStringUtils::extract_filename(file);
    }
}

UTILS_NAMESPACE_END
