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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include <mooon/utils/md5_helper.h>
#include <mooon/utils/charset_utils.h>

int main(int argc, char* argv[])
{
    std::string md5;

    if (argc < 2)
    {
        fprintf(stderr, "usage: md5 string\n");
        exit(1);
    }

    md5 = mooon::utils::CMd5Helper::lowercase_md5("%s", argv[1]);
    printf("%s\n", md5.c_str());

    md5 = mooon::utils::CMd5Helper::uppercase_md5("%s", argv[1]);
    printf("%s\n", md5.c_str());
    return 0;
}
