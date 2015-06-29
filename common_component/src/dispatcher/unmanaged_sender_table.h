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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_DISPATCHER_UNMANAGED_SENDER_TABLE_H
#define MOOON_DISPATCHER_UNMANAGED_SENDER_TABLE_H
#include <mooon/net/ip_node.h>
#include <mooon/utils/hash_map.h>
#include "sender_table.h"
#include "unmanaged_sender.h"
DISPATCHER_NAMESPACE_BEGIN

class CDispatcherContext;
class CUnmanagedSenderTable: public IUnmanagedSenderTable, public CSenderTable
{
public:
    ~CUnmanagedSenderTable();
    CUnmanagedSenderTable(CDispatcherContext* context);    
    
private: // CSenderTable
    virtual void close_sender(CSender* sender);

private: // IUnmanagedSenderTable
    virtual ISender* open_sender(const SenderInfo& sender_info);
    virtual void close_sender(ISender* sender);    
    virtual void release_sender(ISender* sender);
    virtual void remove_sender(ISender* sender);
    virtual ISender* get_sender(const net::ip_node_t& ip_node);  
    
private:
    void clear_sender();

private:
    typedef net::ip_hash_map<CUnmanagedSender*> SenderMap;
    sys::CLock _lock;
    SenderMap _sender_map;
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_UNMANAGED_SENDER_TABLE_H
