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
#include "receiver_factory.h"
MOOON_NAMESPACE_BEGIN

/** 创建包处理器 */
IPacketHandler* CReceiverFactory::create_packet_handler();
{
    return new CPacketHandler;
}

/** 创建协议解析器 */
IProtocolParser* CReceiverFactory::create_protocol_parser();    
{
    return new CProtocolParser;
}

/** 创建请求响应 */
IRequestResponsor* CReceiverFactory::create_request_responsor(IProtocolParser* parser)
{
    return new CRequestResponsor(static_cast<CProtocolParser*>(parser));
}

MOOON_NAMESPACE_END
#endif // MOOON_RECEIVER_H
