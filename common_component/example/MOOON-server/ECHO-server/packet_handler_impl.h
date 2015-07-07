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
#ifndef MOOON_ECHO_SERVER_PACKET_HANDLER_IMPL_H
#define MOOON_ECHO_SERVER_PACKET_HANDLER_IMPL_H
#include <mooon/server/server.h>

class CPakcetHandlerImpl: public mooon::server::IPacketHandler
{
public:    
    CPakcetHandlerImpl(mooon::server::IConnection* connection);
    ~CPakcetHandlerImpl();

private:  
    virtual void reset();
    virtual mooon::utils::handle_result_t on_handle_request(size_t data_size, mooon::server::Indicator& indicator);
    virtual mooon::utils::handle_result_t on_response_completed(mooon::server::Indicator& indicator);

private:
    mooon::server::IConnection* _connection; // 建立的连接
};

#endif // MOOON_ECHO_SERVER_PACKET_HANDLER_IMPL_H
