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
#include "sender_table.h"
DISPATCHER_NAMESPACE_BEGIN

bool check_sender_info(const SenderInfo& sender_info)
{
    //sender_info.key
    //sender_info.ip_node
    //sender_info.queue_size
    //sender_info.resend_times
    //sender_info.reconnect_times
    //sender_info.reply_handler

    if (0 == sender_info.queue_size)
    {
        DISPATCHER_LOG_ERROR("Invalid %s.\n", sender_info_tostring(sender_info).c_str());
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// CSenderTable
CSenderTable::CSenderTable(CDispatcherContext* context)
    :_context(context)
{   
}

CDispatcherContext* CSenderTable::get_context()
{
    return _context;
}

DISPATCHER_NAMESPACE_END
