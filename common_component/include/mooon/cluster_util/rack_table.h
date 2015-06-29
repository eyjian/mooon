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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_CLUSTER_UTIL_RACK_TABLE_H
#define MOOON_CLUSTER_UTIL_RACK_TABLE_H
#include <stdexcept>
#include "cluster_util/rack.h"
#include "sys/read_write_lock.h"
MOOON_NAMESPACE_BEGIN

/** 机架表，创建和管理所有机架
  * 线程安全类
  */
class CRackTable
{
public:
    CRackTable();
    ~CRackTable();
    
    /** 判断指定的ID的机架是否存在
      * @return: 如果机架存在返回true，否则返回false
      */
    bool rack_exist(uint32_t rack_id);

    /** 根据机架ID和IP增加一个机架
      * @exception: 如果rack_id大于RACK_ID_MAX，则抛出range_error异常
      * @return: 成功返回指向CRack的指针，否则如果机架已经存在则返回NULL
      */
    CRack* add_rack(uint32_t rack_id);
    
    /** 删除节点
      * @exception: 如果rack_id大于RACK_ID_MAX，则抛出range_error异常
      */
    void del_rack(CRack* rack);

private:
    sys::CReadWriteLock _lock;
    uint32_t _rack_number; /** 实际机架个数 */
    CRack** _rack_array;   /** 存放机架指针的数组 */
};

MOOON_NAMESPACE_END
#endif // MOOON_CLUSTER_UTIL_RACK_TABLE_H
