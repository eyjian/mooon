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
#ifndef MOOON_SYS_SINGLETON_H
#define MOOON_SYS_SINGLETON_H
#include "mooon/sys/lock.h"
#include <cstdlib>
SYS_NAMESPACE_BEGIN

/** 
 * utils/configh.h 中的 singleton(SINGLETON_DECLARE) 是非线程安全
 * 这里实现的 singleton 是线程安全的
 */

template<typename ObjType>
class CSingleton
{
public:
    static ObjType* get_singleton()
    {
        if (NULL == _obj)
        {
            LockHelper<CLock> monitor(_lock);

            if (NULL == _obj)
            {
                _obj = new ObjType;
                std::atexit(delete_singleton);
            }
        }

        return _obj;
    }

private:
    static void delete_singleton(void)
    {
        if (NULL != _obj)
        {
            delete _obj;
            _obj = NULL;
        }
    }

private:
    static CLock _lock;
    static ObjType* _obj;
};

template <typename ObjType>
CLock CSingleton<ObjType>::_lock;

template <typename ObjType>
ObjType* CSingleton<ObjType>::_obj = NULL;

SYS_NAMESPACE_END

#endif

