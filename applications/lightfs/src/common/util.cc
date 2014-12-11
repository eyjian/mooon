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
#include "common/util.h"

#include <assert.h>
#include <sys/time.h>
namespace lightfs {
namespace common {

void Util::split_string(const std::string& str, char delim,
    std::vector<std::string>* result)
{
    assert(result != NULL);
    const char* p = str.c_str();
    const char* end = p + strlen(p);
    while (p != end) {
        if (*p == delim) {
            ++p;
        } else {
            const char* start = p;
            while (++p != end && *p != delim) {}
            result->push_back(std::string(start, p - start));
        }
    }
}

int32_t Util::read_line(FILE* fp, char* buffer, int32_t max_size)
{
    if (fp == NULL)
        return -1;

    if (!buffer || max_size <= 0)
        return -1;

    int64_t org_offest = ftello(fp);
    if (org_offest < 0)
        return -1;
    char* readed_buffer = fgets(buffer, max_size, fp);
    if (readed_buffer == NULL) {
        if (feof(fp)) {
            return 0;
        } else {
            return -1;
        }
    } else {
        int64_t new_offset = ftello(fp);
        if (new_offset < 0)
            return -1;
        return new_offset - org_offest;
    }
}

int64_t Util::milli_seconds()
{
    struct timeval now;
    gettimeofday(&now, 0);
    return static_cast<uint64_t>(now.tv_sec * 1000) + static_cast<uint64_t>(now.tv_usec / 1000);
}

} // namespace common
} // namespace lightfs
