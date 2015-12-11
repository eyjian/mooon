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
 * Writed on 2015/7/16
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#include "utils/aes_helper.h"
#if HAVE_OPENSSL == 1
#   include <openssl/aes.h>
#endif // HAVE_OPENSSL
UTILS_NAMESPACE_BEGIN

#if HAVE_OPENSSL == 1
int CAESHelper::aes_block_size = AES_BLOCK_SIZE;
#else
int CAESHelper::aes_block_size = 0;
#endif // HAVE_OPENSSL

CAESHelper::CAESHelper(const std::string& key)
{
#if HAVE_OPENSSL == 1
    _aes_key = new AES_KEY;
    AES_set_encrypt_key((const unsigned char*)(key.data()), (int)(key.size()*8), (AES_KEY*)_aes_key);
#else
    _aes_key = NULL;
#endif // HAVE_OPENSSL
}

CAESHelper::~CAESHelper()
{
#if HAVE_OPENSSL == 1
    delete (AES_KEY*)_aes_key;
#endif // HAVE_OPENSSL
}

void CAESHelper::aes(bool flag, const std::string& in, std::string* out)
{
#if HAVE_OPENSSL == 1
    AES_KEY* aes_key = (AES_KEY*)_aes_key;
    const char* in_p = in.data();

    for (std::string::size_type i=0; i<in.size(); i+=AES_BLOCK_SIZE)
    {
        char out_tmp[AES_BLOCK_SIZE];
        if (flag)
            AES_encrypt((const unsigned char*)(in_p), (unsigned char*)(out_tmp), aes_key);
        else
            AES_decrypt((const unsigned char*)(in_p), (unsigned char*)(out_tmp), aes_key);

        in_p += AES_BLOCK_SIZE;
        out->insert(i, out_tmp, AES_BLOCK_SIZE);
    }
#endif // HAVE_OPENSSL
}

UTILS_NAMESPACE_END
