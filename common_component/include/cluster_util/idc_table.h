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
#ifndef MOOON_CLUSTER_UTIL_IDC_TABLE_H
#define MOOON_CLUSTER_UTIL_IDC_TABLE_H
#include <stdexcept>
#include "cluster_util/idc.h"
#include "sys/read_write_lock.h"
MOOON_NAMESPACE_BEGIN

/** IDC表，创建和管理所有IDC
  * 线程安全类
  */
class CIDCTable
{
public:
    CIDCTable();
    ~CIDCTable();
    
    /** 判断指定的ID的IDC是否存在
      * @return: 如果IDC存在返回true，否则返回false
      */
    bool idc_exist(uint32_t idc_id);

    /** 根据IDC ID和IP增加一个IDC
      * @exception: 如果idc_id大于IDC_ID_MAX，则抛出range_error异常
      * @return: 成功返回指向CIDC的指针，否则如果IDC已经存在则返回NULL
      */
    CIDC* add_idc(uint32_t idc_id);
    
    /** 删除IDC
      * @exception: 如果idc_id大于IDC_ID_MAX，则抛出range_error异常
      */
    void del_idc(CIDC* idc);

private:
    sys::CReadWriteLock _lock;
    uint32_t _idc_number; /** 实际IDC个数 */
    CIDC** _idc_array;    /** 存放IDC指针的数组 */
};

MOOON_NAMESPACE_END
#endif // MOOON_CLUSTER_UTIL_IDC_TABLE_H
