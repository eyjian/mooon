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
#ifndef MOOON_RECEIVER_FACTORY_H
#define MOOON_RECEIVER_FACTORY_H
#include <server/server.h>
#include "packet_handler.h"
#include "protocol_parser.h"
#include "request_responsor.h"
MOOON_NAMESPACE_BEGIN

class CReceiverFactory: public IServerFactory
{
private:    
    /** 创建包处理器 */
    virtual IPacketHandler* create_packet_handler();

    /** 创建协议解析器 */
    virtual IProtocolParser* create_protocol_parser();    

    /** 创建请求响应 */
    virtual IRequestResponsor* create_request_responsor(IProtocolParser* parser);
};

MOOON_NAMESPACE_END
#endif // MOOON_RECEIVER_FACTORY_H
