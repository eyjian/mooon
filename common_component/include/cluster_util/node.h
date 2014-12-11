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
#ifndef MOOON_CLUSTER_UTIL_NODE_H
#define MOOON_CLUSTER_UTIL_NODE_H
#include "util/util_config.h"
#include "sys/ref_countable.h"

/** 无效的节点ID值 */
#define INVALID_NODE_ID 0xFFFFFFFF

/** 受控节点ID可取的最大值(含等于)，通常为集群类的节点 */
#define MANAGED_NODE_ID_MAX   100000

/** 非受控节点的最大个数，通常为集群外节点，其ID从MANAGED_NODE_ID_MAX+1开始 */
#define UNMANAGED_NODE_NUMBER 1000

/** 最大节点ID，节点数不能超过最大节点ID值，NODE_ID_MAX的值必须小于INVALID_NODE_ID */
#define NODE_ID_MAX (MANAGED_NODE_ID_MAX+UNMANAGED_NODE_NUMBER)

/** 无效的IDC ID值 */
#define INVALID_IDC_ID 0xFFFFFFFF
/** IDC ID可取的最大值，IDC数不能超过最大IDC ID值，IDC_ID_MAX的值必须小于INVALID_IDC_ID */
#define IDC_ID_MAX 100

/** 无效的机架ID值 */
#define INVALID_RACK_ID 0xFFFFFFFF
/** 机架ID可取的最大值，机架数不能超过最大节点ID值，RACK_ID_MAX的值必须小于INVALID_RACK_ID */
#define RACK_ID_MAX 1000

MOOON_NAMESPACE_BEGIN

/** 判断是否为有效节点ID系列函数 */
bool is_valid_node_id(int id);
bool is_valid_node_id(uint32_t id);

/** 判断是否为有效机架ID系列函数 */
bool is_valid_rack_id(int id);
bool is_valid_rack_id(uint32_t id);

/** 判断是否为有效IDC ID系列函数 */
bool is_valid_idc_id(int id);
bool is_valid_idc_id(uint32_t id);

class CNode: public sys::CRefCountable
{
public:
    CNode(uint32_t id, uint32_t ip, bool managed);
    uint32_t get_id() const { return _id; }
    uint32_t get_ip() const { return _ip; }
    uint32_t get_owner_idc_id() const { return _owner_idc_id; }
    uint32_t get_owner_rack_id() const { return _owner_rack_id; }
    void set_owner_idc_id(uint32_t owner_idc_id) { _owner_idc_id = owner_idc_id; }
    void set_owner_rack_id(uint32_t owner_rack_id) { _owner_rack_id = owner_rack_id; }

    /** 判断节点是否受控 */
    bool is_managed() const { return _managed; }

    /** 判断节点是否活着 */
    bool is_active() const { return (_boot_timestamp > _stop_timestamp); }    
    time_t get_boot_timestamp() const { return _boot_timestamp; }
    time_t get_stop_timestamp() const { return _stop_timestamp; }
    void update_boot_timestamp(time_t boot_timestamp) { _boot_timestamp = boot_timestamp; }
    void update_stop_timestamp(time_t stop_timestamp) { _stop_timestamp = stop_timestamp; }

private:
    uint32_t _id;
    uint32_t _ip;
    uint32_t _owner_idc_id;  /** 节点录属的IDC */
    uint32_t _owner_rack_id; /** 节点录属的机架 */
    bool _managed;           /* 节点类型：是否受控 */

private: // 状态值    
    volatile time_t _boot_timestamp; /** 节点起来时间 */
    volatile time_t _stop_timestamp; /** 节点停止时间 */
};

MOOON_NAMESPACE_END
#endif // MOOON_CLUSTER_UTIL_NODE_H
