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
#include <net/util.h>
#include <sys/close_helper.h>
#include <util/string_util.h>
#include <util/integer_util.h>
#include "dispatcher_context.h"
#include "managed_sender_table.h"
#include "default_reply_handler.h"
DISPATCHER_NAMESPACE_BEGIN

CManagedSenderTable::~CManagedSenderTable()
{
    //clear_sender();
    delete []_sender_table;
    delete []_lock_array;
}

CManagedSenderTable::CManagedSenderTable(CDispatcherContext* context)
    :CSenderTable(context)
{
    _table_size = std::numeric_limits<uint16_t>::max();

    _lock_array = new sys::CLock[_table_size];
    _sender_table = new CManagedSender*[_table_size];
    
    for (int i=0; i<_table_size; ++i)
    {
        _sender_table[i] = NULL;
    }
}

void CManagedSenderTable::close_sender(CSender* sender)
{
    ISender* sender_ = sender;
    CManagedSenderTable::close_sender(sender_);
}

ISender* CManagedSenderTable::open_sender(const SenderInfo& sender_info)
{    
    if (!check_sender_info(sender_info))
    {
        return NULL;
    }

    CManagedSender* sender = NULL;
    sys::LockHelper<sys::CLock> lock(_lock_array[sender_info.key]);

    if (NULL == _sender_table[sender_info.key])
    {    
        sender = new CManagedSender(sender_info);            
        sender->inc_refcount();
        sender->set_in_table(true);

        sender->attach_sender_table(this);
        sender_info.reply_handler->attach(sender);
                
        _sender_table[sender_info.key] = sender;    
        get_context()->add_sender(sender);
    }

    return sender;
}

void CManagedSenderTable::close_sender(ISender* sender)
{
    CManagedSender* sender_ = static_cast<CManagedSender*>(sender);
    uint16_t key = sender_->get_sender_info().key;

    sys::LockHelper<sys::CLock> lock(_lock_array[key]);                
    if (sender_->is_in_table())
    {
        sender_->shutdown();
        sender_->set_in_table(false);
        _sender_table[key] = NULL;
    }

    (void)sender_->dec_refcount();
}

void CManagedSenderTable::release_sender(ISender* sender)
{   
    CManagedSender* sender_ = static_cast<CManagedSender*>(sender);
    uint16_t key = sender_->get_sender_info().key;

    sys::LockHelper<sys::CLock> lock(_lock_array[key]);    
    if (sender_->is_in_table() && sender_->dec_refcount())
    {
        // 因为走到这里，说明Sender已经被deleted
        // ，所以没必要再sender_->set_in_table(false);
        _sender_table[key] = NULL;
    }
}

void CManagedSenderTable::remove_sender(ISender* sender)
{   
    CManagedSender* sender_ = static_cast<CManagedSender*>(sender);
    uint16_t key = sender_->get_sender_info().key;
    
    sys::LockHelper<sys::CLock> lock(_lock_array[key]);                
    if (sender_->is_in_table())
    {
        sender_->set_in_table(false);
        _sender_table[key] = NULL;
    }

    (void)sender_->dec_refcount();
}

ISender* CManagedSenderTable::get_sender(uint16_t key)
{
    CManagedSender* sender = NULL;
    sys::LockHelper<sys::CLock> lock(_lock_array[key]);

    if (_sender_table[key] != NULL)
    {
        sender = _sender_table[key];
        sender->inc_refcount();
    }

    return sender;
}

void CManagedSenderTable::clear_sender()
{
    // 下面这个循环最大可能为65535次，但只有更新发送表时才发生，所以对性能影响可以忽略    
    for (uint16_t key=0; key<_table_size; ++key)
    {
        sys::LockHelper<sys::CLock> lock(_lock_array[key]);
        if (_sender_table[key] != NULL)
        {
            _sender_table[key]->shutdown();
            _sender_table[key]->dec_refcount();
            _sender_table[key] = NULL;
        }
    }
}

DISPATCHER_NAMESPACE_END
