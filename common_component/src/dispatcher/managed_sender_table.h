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
#ifndef MOOON_DISPATCHER_MANAGED_SENDER_TABLE_H
#define MOOON_DISPATCHER_MANAGED_SENDER_TABLE_H
#include "sender_table.h"
#include "managed_sender.h"
DISPATCHER_NAMESPACE_BEGIN

class CDispatcherContext;
class CManagedSenderTable: public IManagedSenderTable, public CSenderTable
{        
    typedef CManagedSender** sender_table_t;
    
public:
    ~CManagedSenderTable();
    CManagedSenderTable(CDispatcherContext* context);               

private: // CSenderTable
    virtual void close_sender(CSender* sender);

private: // IManagedSenderTable 
    virtual ISender* open_sender(const SenderInfo& sender_info);
    virtual void close_sender(ISender* sender);    
    virtual void release_sender(ISender* sender);
    virtual void remove_sender(ISender* sender);
    virtual ISender* get_sender(uint16_t key);

private:
    void clear_sender();

private:        
    uint16_t _table_size;
    sys::CLock* _lock_array;
    sender_table_t _sender_table;        
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_MANAGED_SENDER_TABLE_H
