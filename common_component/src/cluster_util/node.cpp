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
#include "cluster_util/node.h"
MOOON_NAMESPACE_BEGIN

CNode::CNode(uint32_t id, uint32_t ip, bool managed)
    :_id(id)
    ,_ip(ip)
    ,_owner_idc_id(INVALID_IDC_ID)
    ,_owner_rack_id(INVALID_RACK_ID)
    ,_managed(managed)
    ,_boot_timestamp(0)
    ,_stop_timestamp(0)
{
}

//////////////////////////////////////////////////////////////////////////
// 全局函数

// 节点
bool is_valid_node_id(int id)
{
    return (id <= NODE_ID_MAX) && (id >= 0);
}

bool is_valid_node_id(uint32_t id)
{
    return id <= NODE_ID_MAX;
}

// 机架
bool is_valid_rack_id(int32_t id)
{
    return (id <= RACK_ID_MAX) && (id >= 0);
}

bool is_valid_rack_id(uint32_t id)
{
    return id <= RACK_ID_MAX;
}

// IDC
bool is_valid_idc_id(int32_t id)
{
    return (id <= IDC_ID_MAX) && (id >= 0);
}

bool is_valid_idc_id(uint32_t id)
{
    return id <= IDC_ID_MAX;
}

MOOON_NAMESPACE_END
