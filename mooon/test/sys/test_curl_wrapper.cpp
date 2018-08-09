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
 * Author: eyjian@qq.com or eyjian@gmail.com or eyjian@live.com
 */
#include <mooon/sys/curl_wrapper.h>
#include <mooon/utils/string_utils.h>
MOOON_NAMESPACE_USE

extern "C" int main(int argc, char* argv[])
{
    bool enable_insecure = false;
    bool nosignal = false;
    int min = 2, max = 5;
    int data_timeout_seconds = 5;
    int connect_timeout_seconds = 2;
    std::string url;

    if ((argc > 1) && (0 == strcmp(argv[1], "-k")))
    {
        enable_insecure = true;
        min = 3;
        max = 6;
    }
    if ((argc < min) || (argc > max))
    {
        fprintf(stderr, "Usage1: curl_get <-k> url, e.g., curl_get 'http://www.qq.com'\n");
        fprintf(stderr, "Usage2: curl_get <-k> url data_timeout_seconds, e.g., curl_get 'http://www.abc123.com' 10\n");
        fprintf(stderr, "Usage3: curl_get <-k> url data_timeout_seconds connect_timeout_seconds, e.g., curl_get 'http://www.abc123.com' 10 20\n");
        fprintf(stderr, "Usage4: curl_get <-k> url data_timeout_seconds connect_timeout_seconds nosignal, e.g., curl_get 'http://www.abc123.com' 10 20 yes\n");
        exit(1);
    }

    if (!enable_insecure)
    {
        url = argv[1];
        if (argc > 2)
            data_timeout_seconds = mooon::utils::CStringUtils::string2int<int>(argv[2]);
        if (argc > 3)
            connect_timeout_seconds = mooon::utils::CStringUtils::string2int<int>(argv[3]);
        if (argc > 4)
            nosignal = (0 == strcmp(argv[4], "yes"));
    }
    else
    {
        url = argv[2];
        if (argc > 2)
            data_timeout_seconds = mooon::utils::CStringUtils::string2int<int>(argv[3]);
        if (argc > 3)
            connect_timeout_seconds = mooon::utils::CStringUtils::string2int<int>(argv[4]);
        if (argc > 4)
            nosignal = (0 == strcmp(argv[5], "yes"));
    }

    try
    {
        sys::CCurlWrapper curl_wrapper(data_timeout_seconds, connect_timeout_seconds, nosignal);
        std::string response_header;
        std::string response_body;

        curl_wrapper.http_get(response_header, response_body, url.c_str());
        printf("result =>\n%s\n", response_body.c_str());

        response_header.clear();
        response_body.clear();
        curl_wrapper.http_get(response_header, response_body, url.c_str());
        printf("result =>\n%s\n", response_body.c_str());
    }
    catch (utils::CException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }
    
    return 0;
}
