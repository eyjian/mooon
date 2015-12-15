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
#include "mooon/utils/config.h"
UTILS_NAMESPACE_BEGIN

class CAESHelper
{
public:
    static int aes_block_size;

public:
    CAESHelper(const std::string& key);
    ~CAESHelper();

    void encrypt(const std::string& in, std::string* out);
    void decrypt(const std::string& in, std::string* out);

private:
    // flag 为true表示加密，为false表示解密
    void aes(bool flag, const std::string& in, std::string* out, void* aes_key);

private:
    void* _encrypt_key;
    void* _decrypt_key;
    std::string _key;
};

UTILS_NAMESPACE_END
