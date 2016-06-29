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
 * Author: caobiao@gmail.com
 */
#ifndef LIGHTFS_COMMON_UTIL_H
#define LIGHTFS_COMMON_UTIL_H
#include <fcntl.h>
#include <string>
#include <vector>
namespace lightfs {
namespace common {

class Util
{
public:
    static void split_string(const std::string& str, char delim,
        std::vector<std::string>* result);

    static int32_t read_line(FILE* fp, char* buffer, int32_t max_size);

    static int64_t milli_seconds();
};

} // namespace common
} // namespace lightfs
#endif // LIGHTFS_COMMON_UTIL_H

