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

// 传递给do_convert的in_buf，所有字节数（in_buf_size指定）都是可以转换成功的
static int do_convert(iconv_t cd, const char* from, size_t from_size, std::string* to)
{
    char* in_buf_ptr = const_cast<char*>(from);
    size_t in_bytes_left = from_size;
    size_t out_bytes = in_bytes_left*3 + 1;
    size_t out_bytes_left = out_bytes;
    std::string out(out_bytes_left, '\0');
    char* out_buf_start = const_cast<char*>(out.c_str());
    char* out_buf_ptr = out_buf_start;

    int bytes = iconv(cd, &in_buf_ptr, &in_bytes_left, &out_buf_ptr, &out_bytes_left);
    if (-1 == bytes)
        return errno;

    to->assign(out_buf_start, out_bytes-out_bytes_left);
    return 0;
}

void CCharsetUtils::convert(const std::string& from_charset, const std::string& to_charset,
                            const std::string& from, std::string* to,
                            bool ignore_error, bool skip_error) throw (CException)
{
    std::string result; // 用来保存处理后的内容
    char* in_buf = const_cast<char*>(from.c_str());
    size_t in_bytes = from.size();   // 需要处理的总字节数
    size_t in_bytes_left = in_bytes; // 剩余的未被处理的字节数
    iconv_t cd = iconv_open(to_charset.c_str(), from_charset.c_str());

    if ((iconv_t)(-1) == cd)
    {
        THROW_EXCEPTION(strerror(errno), errno);
    }
    while (in_bytes_left > 0)
    {
        int errcode;
        size_t out_bytes = in_bytes_left * 3 + 1; // 保证足够大
        size_t out_bytes_left = out_bytes;
        std::string out(out_bytes_left, '\0');
        char* out_buf = const_cast<char*>(out.c_str());
        char* out_buf_start = out_buf;
        char* in_buf_start = in_buf;

        // 如果成功，返回值bytes为0
        // 如果成功，in_buf指向in的结尾符，即'\0'，同时in_bytes_left值为0
        // 如果失败，in_buf指向未能转换的起始地址，而in_bytes_left值为剩余的未被转换的（可能含有可转换的）字节数
        // 如果成功，则out_bytes-out_bytes_left值为转换后的字节数
        // 如果成功，则out_buf_start存储了被转换后的结果，有效长度为out_bytes-out_bytes_left
        int bytes = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
        if (bytes != -1)
        {
            result.append(out_buf_start, out_bytes-out_bytes_left);
            break;
        }
        else if (!ignore_error)
        {
            errcode = errno;
            iconv_close(cd);
            THROW_EXCEPTION(strerror(errcode), errcode);
        }
        else
        {
            // EILSEQ An invalid multibyte sequence has been encountered in the input.
            // EINVAL An incomplete multibyte sequence has been encountered in the input.
            if ((errno != EINVAL) &&
                (errno != EILSEQ))
            {
                // E2BIG  There is not sufficient room at *outbuf.
                errcode = errno;
                iconv_close(cd);
                THROW_EXCEPTION(strerror(errcode), errcode);
            }
            else
            {
                // in_buf之前部分是可以转换的
                if (in_buf != in_buf_start)
                {
                    std::string str;
                    errcode = do_convert(cd, in_buf_start, in_buf-in_buf_start, &str);
                    if (errcode != 0)
                    {
                        iconv_close(cd);
                        THROW_EXCEPTION(strerror(errcode), errcode);
                    }

                    result.append(str);
                }

                // skip_error决定未能被转换的是否出现在结果当中
                if (!skip_error)
                {
                    result.append(in_buf, 1);
                }

                // 往前推进
                --in_bytes_left; // 将导致while语句结束
                ++in_buf;
            }
        }
    }

    if (-1 == iconv_close(cd))
    {
        THROW_EXCEPTION(strerror(errno), errno);
    }

    // 不能直接使用to，因为to可能就是from
    *to = result;
}

void CCharsetUtils::gbk_to_utf8(const std::string& from, std::string* to, bool ignore_error, bool skip_error) throw (CException)
{
    convert("gbk", "utf-8", from, to, ignore_error, skip_error);
}

void CCharsetUtils::utf8_to_gbk(const std::string& from, std::string* to, bool ignore_error, bool skip_error) throw (CException)
{
    convert("utf-8", "gbk", from, to, ignore_error, skip_error);
}

void CCharsetUtils::gb2312_to_utf8(const std::string& from, std::string* to, bool ignore_error, bool skip_error) throw (CException)
{
    convert("gb2312", "utf-8", from, to, ignore_error, skip_error);
}

void CCharsetUtils::utf8_to_gb2312(const std::string& from, std::string* to, bool ignore_error, bool skip_error) throw (CException)
{
    convert("utf-8", "gb2312", from, to, ignore_error, skip_error);
}

UTILS_NAMESPACE_END
