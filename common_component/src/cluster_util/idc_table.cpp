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
#include "cluster_util/idc_table.h"
MOOON_NAMESPACE_BEGIN

CIDCTable::CIDCTable()
{
    _idc_array = new CIDC*[IDC_ID_MAX];
    for (uint32_t i=0; i<IDC_ID_MAX; ++i)
        _idc_array[i] = NULL;
}

CIDCTable::~CIDCTable()
{
    for (uint32_t i=0; i<IDC_ID_MAX; ++i)    
        delete _idc_array[i];
    
    delete []_idc_array;
}

bool CIDCTable::idc_exist(uint32_t idc_id)
{    
    sys::ReadLockHelper read_lock(_lock);
    return is_valid_idc_id(idc_id)? (_idc_array[idc_id] != NULL): false;
}

CIDC* CIDCTable::add_idc(uint32_t idc_id)
{        
    if (!is_valid_idc_id(idc_id)) return NULL;
    sys::WriteLockHelper write_lock(_lock);

    // 节点已经存在
    if (_idc_array[idc_id] != NULL) return NULL;
        
    _idc_array[idc_id] = new CIDC(idc_id);
    _idc_array[idc_id]->inc_refcount();
    return _idc_array[idc_id];
}

void CIDCTable::del_idc(CIDC* idc)
{
    uint32_t idc_id = idc->get_id();
    if (is_valid_idc_id(idc_id))
    {    
        sys::WriteLockHelper write_lock(_lock);
        if (_idc_array[idc_id] != NULL)
        {        
            //delete _idc_array[idc_id];
            _idc_array[idc_id]->dec_refcount();
            _idc_array[idc_id] = NULL;
        }
    }
}

MOOON_NAMESPACE_END
