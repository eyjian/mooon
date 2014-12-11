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
#ifndef MOOON_UTIL_FILE_FORMAT_EXCEPTION_H
#define MOOON_UTIL_FILE_FORMAT_EXCEPTION_H
#include "util/config.h"
UTIL_NAMESPACE_BEGIN

class CFileFormatException
{
public:
    CFileFormatException(const char* filename, int line_number, int field_number=0);
    const char* get_filename() const { return _filename.c_str(); }
    int get_line_number() const { return _line_number; }
    int get_field_number() const { return _field_number; }

private:
    std::string _filename; /** 格式存在错误的文件名 */
    int _line_number;       /** 错误发生的行号 */
    int _field_number;      /** 错误发生的列号或字段号 */
};

UTIL_NAMESPACE_END
#endif // MOOON_UTIL_FILE_FORMAT_EXCEPTION_H
