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
#include <sys/logger.h>
#include <sys/sys_util.h>
#include <server/server.h>
#include "general_config.h"
#include "general_factory.h"
int main()
{    
    sys::CLogger* logger = new sys::CLogger;
    logger->create("../log", "general.log");
    sys::g_logger = logger;

    mooon::CGeneralConfig* config = new mooon::CGeneralConfig;
    mooon::CGeneralFactory* factory = new mooon::CGeneralFactory;
    
    mooon::IServer* server = create_server(logger, config, factory);
    server->start();

    sys::CSysUtil::millisleep(1000000);
    server->stop();
    destroy_server(server);
    return 0;
}
