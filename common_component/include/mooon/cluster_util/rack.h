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
#ifndef MOOON_CLUSTER_UTIL_RACK_H
#define MOOON_CLUSTER_UTIL_RACK_H
#include "cluster_util/node.h"
MOOON_NAMESPACE_BEGIN

class CRack: public sys::CRefCountable
{
public:
    CRack(uint32_t id);
    uint32_t get_id() const { return _id; }    
    uint32_t get_owner_idc_id() const { return _owner_idc_id; }
    const std::string& get_name() const { return _name; }
    void set_name(const std::string& name) { _name = name; }
    void set_owner_idc_id(uint32_t owner_idc_id) { _owner_idc_id = owner_idc_id; }

private:
    uint32_t _id;
    uint32_t _owner_idc_id;  /** 机架录属的IDC */
    std::string _name;
};

MOOON_NAMESPACE_END
#endif // MOOON_CLUSTER_UTIL_RACK_H
