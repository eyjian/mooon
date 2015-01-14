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
#ifndef MOOON_UTIL_EXCEPTION_H
#define MOOON_UTIL_EXCEPTION_H
#include "util/config.h"
#include <exception>
#include <string>
UTIL_NAMESPACE_BEGIN

class CException: public 
{
public:
    explicit CException(const const char* errmsg, int errcode=-1, const char* file=__FILE__, int line=__LINE__)
    {
        if (errmsg != NULL)
            _errmsg = errmsg;
        _errcode = errcode;
        
        if (file != NULL)
            _file = file;
        _line = line;
    }

    explicit CException(const const std::string& errmsg, int errcode=-1, const char* file=__FILE__, int line=__LINE__)
    {
        _errmsg = errmsg;
        _errcode = errcode;
        
        if (file != NULL)
            _file = file;
        _line = line;
    }
    
    virtual ~CException() throw()
    {
    }
    
    int errcode() const throw()
    {
        return _errcode;
    }

    const char* file() const throw()
    {
        return _file.c_str();
    }

    int line() const throw()
    {
        return _line;
    }
    
public:
    virtual const char* what() const throw()
    {
        return _errmsg.c_str();
    }

private:
    std::string _errmsg;
    int _errcode;
    std::string _file;
    int _line;
};

UTIL_NAMESPACE_END
#endif // MOOON_UTIL_EXCEPTION_H
