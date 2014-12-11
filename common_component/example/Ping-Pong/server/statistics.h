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
 * Author: laocai_liu@qq.com or laocailiu@gmail.com
 */

#ifndef MOOON_PP_STATISTICS_H
#define MOOON_PP_STATISTICS_H

#include <sys/singleton.h>
#include "pp_log.h"

PP_NAMESPACE_BEGIN

class CStatistics : public sys::CSingleton<CStatistics>
{
public:
    CStatistics()
    {
        _pp_msg_count = 0;
        _to_show_max_count = 0;
    }

    void inc_pp_msg_count(void)
    {
        ++_pp_msg_count;
    }
    uint64_t pp_msg_count(void)
    {
        return _pp_msg_count;
    }

private:
    uint64_t _pp_msg_count;
    unsigned int _to_show_max_count;
};

PP_NAMESPACE_END

#endif

