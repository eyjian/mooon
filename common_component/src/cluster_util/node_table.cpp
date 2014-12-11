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
#include "net/net_util.h"
#include "sys/CloseHelper.h"
#include "cluster_util/node_table.h"
MOOON_NAMESPACE_BEGIN

CNodeTable::CNodeTable(bool ip_uniq)
    :_ip_uniq(ip_uniq)
{
    _node_array = new CNode*[NODE_ID_MAX];
    for (uint32_t i=0; i<NODE_ID_MAX; ++i)
        _node_array[i] = NULL;
}

CNodeTable::~CNodeTable()
{
    clear_nodes();    
    delete []_node_array;
}

void CNodeTable::clear_nodes()
{    
    for (uint32_t i=0; i<NODE_ID_MAX; ++i)
        do_del_node(_node_array[i]);
}

CNode* CNodeTable::get_node(uint32_t node_id, bool inc_refcount)
{    
    CNode* node = NULL;
    sys::ReadLockHelper read_lock(_lock);    

    if (is_valid_node_id(node_id))
    {
        if (_node_array[node_id] != NULL)
        {
            node = _node_array[node_id];
            if (inc_refcount)
                node->inc_refcount();
        }
    }

    return node;
}

bool CNodeTable::node_exist(uint32_t node_ip)
{
    sys::ReadLockHelper read_lock(_lock);
    ip_table_t::iterator iter = _ip_table.find(node_ip);
    return (iter != _ip_table.end());
}

CNodeTable::node_id_set_t* CNodeTable::get_node_id_set(uint32_t node_ip)
{
    sys::ReadLockHelper read_lock(_lock);
    ip_table_t::iterator iter = _ip_table.find(node_ip);
    if (iter == _ip_table.end()) return NULL;

    return iter->second;
}

CNode* CNodeTable::add_node(uint32_t node_id, uint32_t node_ip, bool managed)
{
    sys::WriteLockHelper write_lock(_lock);
    return do_add_node(node_id, node_ip, managed);
}

CNode* CNodeTable::do_add_node(uint32_t node_id, uint32_t node_ip, bool managed)
{   
    // 无效节点ID
    if (!is_valid_node_id(node_id)) return NULL;

    // 节点已经存在
    if (_node_array[node_id] != NULL) return NULL;
            
    // IP对应的ID存储在set中
    node_id_set_t* node_id_set;
    ip_table_t::iterator iter = _ip_table.find(node_ip);
    if (iter == _ip_table.end())
    {    
        node_id_set = new node_id_set_t;
        node_id_set->insert(node_id);

        _ip_table.insert(std::make_pair(node_ip, node_id_set));
    }
    else
    {
        // 相同IP的已经存在
        if (_ip_uniq) return NULL;
        
        node_id_set = iter->second;
        node_id_set->insert(node_id);
    }

    ++_node_number;
    _node_array[node_id] = new CNode(node_id, node_ip, managed);    
    _node_array[node_id]->inc_refcount();
    return _node_array[node_id];
}

void CNodeTable::del_node(CNode* node)
{
    sys::WriteLockHelper write_lock(_lock);
    do_del_node(node);
}

void CNodeTable::do_del_node(CNode* node)
{
    if (node != NULL)
    {
        uint32_t node_id = node->get_id();
        uint32_t node_ip = node->get_ip();
        
        // 有效节点ID
        if (is_valid_node_id(node_id))
        {   
            if (_node_array[node_id] != NULL)
            {            
                //delete _node_array[node_id];
                _node_array[node_id]->dec_refcount(); // 如果引用计数值为0，则相当于delete
                _node_array[node_id] = NULL;
                --_node_number;

                // 处理IP表
                ip_table_t::iterator iter = _ip_table.find(node_ip);
                if (iter != _ip_table.end())
                {
                    node_id_set_t* node_id_set = iter->second;
                    node_id_set->erase(node_id);
                    if (node_id_set->empty())
                    {
                        _ip_table.erase(iter);
                        delete node_id_set;
                    }
                }
            }
        }
    }
}

void CNodeTable::load(const char* filename, bool ignore_duplicate)
{
    FILE* fp = fopen(filename, "r");
    if (NULL == fp)
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    sys::CloseHelper<FILE*> ch(fp);
    sys::WriteLockHelper write_lock(_lock);

    try
    {        
        int idc_id = 0;
        int node_id = 0;
        int rack_id = 0;        
        int node_type = 0;                
        char node_ip_str[IP_ADDRESS_MAX];
        char check_field[100]; /** 用来探测是否多了一些字段 */
        
        char line[LINE_MAX];
        int line_number = 0;

        while (fgets(line, sizeof(line)-1, fp))
        {
            ++line_number;

            // 跳过注释
            if ('#' == line[0]) continue;
            // 跳过空行
            if ('\n' == line[0]) continue;
 
            // 共5个有效字段
            int field_count = sscanf(line, "%d%s%d%d%d%s", &node_id, node_ip_str, &node_type, &rack_id, &idc_id, check_field);
            if (field_count != 5) // 字段个数不对
                throw util::CFileFormatException(filename, line_number, field_count);

            // 节点ID过大
            if (!is_valid_node_id(node_id))
                throw util::CFileFormatException(filename, line_number, 1);
        
            // IP格式不对
            if (!net::CUtil::valid_ipv4(node_ip_str))
                throw util::CFileFormatException(filename, line_number, 2);

            int node_ip = net::CUtil::convert_ipv4(node_ip_str);
            if (0 == node_ip)
                throw util::CFileFormatException(filename, line_number, 2);
                        
            // 节点类型不对
            if ((node_type != 0) && (node_type != 1))
                throw util::CFileFormatException(filename, line_number, 3);

            // rack error
            if ((rack_id != -1) && (!is_valid_rack_id(rack_id)))
                throw util::CFileFormatException(filename, line_number, 4);

            // idc error
            if ((idc_id != -1) && (!is_valid_idc_id(idc_id)))         
                throw util::CFileFormatException(filename, line_number, 5);

            // node_type等于1表示为受控节点，如果等于0则表示为非控节点
            CNode* node = do_add_node(node_id, node_ip, 1==node_type);         
            if (NULL == node) // 除非节点已经存在，否则不会为NULL
            {
                if (!ignore_duplicate)
                    throw util::CFileFormatException(filename, line_number, 0);
            }
            else
            {            
                if (idc_id > -1)
                    node->set_owner_idc_id(idc_id);
                if (rack_id > -1)
                    node->set_owner_rack_id(rack_id);
            }
        }
    }
    catch (util::CFileFormatException& ex)
    {
        //fclose(fp); // 使用了CloseHelper，会自动关闭的
        clear_nodes();
        throw;
    }
}

MOOON_NAMESPACE_END
