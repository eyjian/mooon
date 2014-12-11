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
#include "cluster_util/rack_table.h"
MOOON_NAMESPACE_BEGIN

CRackTable::CRackTable()
{
    _rack_array = new CRack*[RACK_ID_MAX];
    for (uint32_t i=0; i<RACK_ID_MAX; ++i)
        _rack_array[i] = NULL;
}

CRackTable::~CRackTable()
{
    for (uint32_t i=0; i<RACK_ID_MAX; ++i)    
        delete _rack_array[i];
    
    delete []_rack_array;
}

bool CRackTable::rack_exist(uint32_t rack_id)
{       
    sys::ReadLockHelper read_lock(_lock);
    return is_valid_rack_id(rack_id)? (_rack_array[rack_id] != NULL): false;
}

CRack* CRackTable::add_rack(uint32_t rack_id)
{        
    if (!is_valid_rack_id(rack_id)) return NULL;    
    sys::WriteLockHelper write_lock(_lock);

    // 节点已经存在
    if (_rack_array[rack_id] != NULL) return NULL;
        
    _rack_array[rack_id] = new CRack(rack_id);
    _rack_array[rack_id]->inc_refcount();
    return _rack_array[rack_id];
}

void CRackTable::del_rack(CRack* rack)
{
    uint32_t rack_id = rack->get_id();
    if (is_valid_rack_id(rack_id))
    {    
        sys::WriteLockHelper write_lock(_lock);
        if (_rack_array[rack_id] != NULL)
        {        
            //delete _rack_array[rack_id];
            _rack_array[rack_id]->dec_refcount();
            _rack_array[rack_id] = NULL;
        }
    }
}

MOOON_NAMESPACE_END
