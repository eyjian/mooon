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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_UTILS_TIMEOUTABLE_H
#define MOOON_UTILS_TIMEOUTABLE_H
#include "mooon/utils/config.h"
#include <time.h>
UTILS_NAMESPACE_BEGIN

/***
  * 可超时对象的基类
  * 不应当直接使用此类，而应当总是继承方式
  */
class CTimeoutable
{
public:
    CTimeoutable()
        :_timestamp(0)
    {
    }

    /*** 得到时间戳 */
    time_t get_timestamp() const { return _timestamp; }

    /** 设置新的时间戳 */
    void set_timestamp(time_t timestamp) { _timestamp = timestamp; }

private:
    time_t _timestamp;
};

UTILS_NAMESPACE_END
#endif // MOOON_UTILS_TIMEOUTABLE_H
