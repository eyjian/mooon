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
#ifndef MOOON_CLUSTER_UTIL_NODE_TABLE_H
#define MOOON_CLUSTER_UTIL_NODE_TABLE_H
#include <map>
#include <set>
#include <stdexcept>
#include "cluster_util/node.h"
#include "sys/read_write_lock.h"
#include "sys/syscall_exception.h"
#include "util/file_format_exception.h"
MOOON_NAMESPACE_BEGIN

/** 节点表，创建和管理所有节点
  * 线程安全类
  */
class CNodeTable
{
public:
    /** 节点ID集 */
    typedef std::set<uint32_t> node_id_set_t;

public:
    /** 构造节点表
      * @ip_uniq: ID和IP是否一一对应，即两者是否均为唯一的，不重复
      */
    CNodeTable(bool ip_uniq=true);
    ~CNodeTable();
    
    /** 得到指定ID的节点
      * @node_id: 节点ID
      * @inc_refcount: 是否增加引用计数，如果增加了引用计数，则在使用完后，应当减引用计数
      * @return: 返回指定ID的节点指针，如果节点并不存在，则返回NULL
      */
    CNode* get_node(uint32_t node_id, bool inc_refcount=false);

    /** 根据IP，判断是否有对应的节点存在，
      * 请注意一个IP可对应多个不同的Node
      */
    bool node_exist(uint32_t node_ip);

    /** 通过IP得到其下所有的的节点ID
      */
    node_id_set_t* get_node_id_set(uint32_t node_ip);

    /** 根据节点ID和IP增加一个节点
      * @node_id: 节点ID
      * @node_ip: 节点IP地址
      * @managed: 是否为受控节点
      * @exception: 如果node_id大于NODE_ID_MAX，则抛出range_error异常
      * @return: 成功返回指向CNode的指针，否则如果节点已经存在或节点ID无效则返回NULL
      */
    CNode* add_node(uint32_t node_id, uint32_t node_ip, bool managed);
    
    /** 删除节点
      * @node: 指向需要删除的节点的指针
      * @exception: 如果node_id大于NODE_ID_MAX，则抛出range_error异常
      */
    void del_node(CNode* node);

    /** 从文件中加载节点表，文件每一行格式均为：
      * 节点ID	节点IP	节点类型	节点隶属的机架ID	节点隶属的IDC的ID，各字段以空格或TAB分隔
      * @1: 节点ID不能为负值
      * @2: 节点IP必须为有效的IPV4地址
      * @3: 节点类型只能为0或1，其中1表示为受控节点
      * @4: 节点隶属机架ID为-1表示不设置
      * @5: 节点隶属IDC ID为-1表示不设置
      * @ignore_duplicate: 是否忽略重复项
      * @exception: 文件格式错误会抛出CFileFormatException异常，打开文件出错则抛出CSyscallException异常
      */
    void load(const char* filename, bool ignore_duplicate);
    
    CNode** get_node_array() const { return _node_array; }
    uint32_t get_node_number() const { return _node_number; }

private:
    /** 删除并清空所有节点 */
    void clear_nodes();
    void do_del_node(CNode* node);
    CNode* do_add_node(uint32_t node_id, uint32_t node_ip, bool managed);

private:
    sys::CReadWriteLock _lock;   
    bool _ip_uniq;         /** ID和IP是否一一对应，即两者是否均为唯一的，不重复 */
    CNode** _node_array;   /** 存放节点指针的数组 */
    volatile uint32_t _node_number; /** 实际节点个数 */    
    typedef std::map<uint32_t, node_id_set_t* > ip_table_t;
    ip_table_t _ip_table;
};

MOOON_NAMESPACE_END
#endif // MOOON_CLUSTER_UTIL_NODE_TABLE_H
