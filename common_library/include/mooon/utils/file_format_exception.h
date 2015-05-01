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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_UTILS_FILE_FORMAT_EXCEPTION_H
#define MOOON_UTILS_FILE_FORMAT_EXCEPTION_H
#include "mooon/utils/exception.h"
UTILS_NAMESPACE_BEGIN

class CFileFormatException: public CException
{
public:
    CFileFormatException(const char* file, int line, int field=0);
    int field() const { return _field; }

private:    
    int _field; /** 错误发生的列号或字段号 */
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_FILE_FORMAT_EXCEPTION_H
