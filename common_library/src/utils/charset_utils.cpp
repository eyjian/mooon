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
                 const std::string& from, std::string* to) throw (CException)
{
    iconv_t cd = iconv_open(to_charset.c_str(), from_charset.c_str());
    if ((iconv_t)(-1) == cd)
        THROW_EXCEPTION(strerror(errno), errno);

    size_t inbytesleft = from.size();
    char *inbuf = const_cast<char*>(from.c_str());

    // 保证足够大
    size_t outbytesleft = inbytesleft + inbytesleft;
    char* outbuf = NULL;
    while (true)
    {
        outbuf = new char[outbytesleft];

        if (-1 == iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft))
        {
            int errcode = errno;
            delete []outbuf;

            if (E2BIG == errcode) // There is not sufficient room at *outbuf
            {
                outbytesleft += outbytesleft;
            }
            else
            {
                iconv_close(cd);
                THROW_EXCEPTION(strerror(errcode), errcode);
            }
        }
        else
        {
            break;
        }
    }

    if (-1 == iconv_close(cd))
    {
        delete []outbuf;
        THROW_EXCEPTION(strerror(errno), errno);
    }

    to->assign(outbuf, outbytesleft);
    delete []outbuf;
}

void CCharsetUtils::gbk_to_utf8(const std::string& from, std::string* to) throw (CException)
{
    convert("gbk", "utf-8", from, to);
}

void CCharsetUtils::utf8_to_gbk(const std::string& from, std::string* to) throw (CException)
{
    convert("utf-8", "gbk", from, to);
}

void CCharsetUtils::gb2312_to_utf8(const std::string& from, std::string* to) throw (CException)
{
    convert("gb2312", "utf-8", from, to);
}

void CCharsetUtils::utf8_to_gb2312(const std::string& from, std::string* to) throw (CException)
{
    convert("utf-8", "gb2312", from, to);
}

UTILS_NAMESPACE_END
