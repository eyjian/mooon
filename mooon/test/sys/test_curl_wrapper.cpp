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
MOOON_NAMESPACE_USE

extern "C" int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: curl_get url\n");
        exit(1);
    }
    
    try
    {
        sys::CCurlWrapper curl_wrapper(2);
        std::string response_header;
        std::string response_body;

        curl_wrapper.http_get(response_header, response_body, argv[1]);
        printf("result =>\n%s\n", response_body.c_str());

        response_header.clear();
        response_body.clear();
        curl_wrapper.http_get(response_header, response_body, argv[1]);
        printf("result =>\n%s\n", response_body.c_str());
    }
    catch (utils::CException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }
    
    return 0;
}
