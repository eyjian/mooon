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
#include "utils/charset_utils.h"
UTILS_NAMESPACE_BEGIN

void CCharsetUtils::convert(const std::string& from_charset, const std::string& to_charset,
                 const std::string& from, std::string* to, bool ignore_error) throw (CException)
{
    iconv_t cd = iconv_open(to_charset.c_str(), from_charset.c_str());
    if ((iconv_t)(-1) == cd)
        THROW_EXCEPTION(strerror(errno), errno);

    std::string in = from;
    size_t in_bytes = in.size();
    size_t out_bytes = in_bytes * 3 + 1;
    std::string out(out_bytes, '\0');
    char* in_buf = const_cast<char*>(in.c_str());
    char* out_buf = const_cast<char*>(out.c_str());

    while (true)
    {
        size_t in_bytes_left = in_bytes;
        size_t out_bytes_left = out_bytes;

        // iconv会往后移到in_buf，并递减in_bytes_left
        // 同时，往后移out_buf，并递减out_bytes_left
        int bytes = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
        if (bytes != -1)
        {
            // 注意，不能使用out_buf，因为它被iconv往后移动了
            // out_bytes-out_bytes_left的值等于out_buf-out.c_str()，也就是移动的大小
            to->append(out.c_str(), out_bytes-out_bytes_left);
            break;
        }
        else if (!ignore_error)
        {
            int errcode = errno;
            iconv_close(cd);
            THROW_EXCEPTION(strerror(errcode), errcode);
        }
        else
        {
            // 忽略出错
            to->append(in_buf, 1);

            ++in_buf;
            --in_bytes_left;
            out_buf = const_cast<char*>(out.c_str());
            out_bytes_left = out_bytes;
        }
    }

    if (-1 == iconv_close(cd))
    {
        THROW_EXCEPTION(strerror(errno), errno);
    }
}

void CCharsetUtils::gbk_to_utf8(const std::string& from, std::string* to, bool ignore_error) throw (CException)
{
    convert("gbk", "utf-8", from, to, ignore_error);
}

void CCharsetUtils::utf8_to_gbk(const std::string& from, std::string* to, bool ignore_error) throw (CException)
{
    convert("utf-8", "gbk", from, to, ignore_error);
}

void CCharsetUtils::gb2312_to_utf8(const std::string& from, std::string* to, bool ignore_error) throw (CException)
{
    convert("gb2312", "utf-8", from, to, ignore_error);
}

void CCharsetUtils::utf8_to_gb2312(const std::string& from, std::string* to, bool ignore_error) throw (CException)
{
    convert("utf-8", "gb2312", from, to, ignore_error);
}

UTILS_NAMESPACE_END
