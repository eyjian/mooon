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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef MOOON_SERVER_H
#define MOOON_SERVER_H
#include <mooon/server/factory.h>
SERVER_NAMESPACE_BEGIN

typedef void* server_t;

/**
  * 日志器，所有实例共享同一个日志器
  * 如需要记录日志，则在调用create_server之前应当设置好日志器
  */
extern sys::ILogger* logger;

//////////////////////////////////////////////////////////////////////////

/**
  * 销毁Server
  */
extern void destroy(server_t server);

/***
  * 创建Server
  * @config: Server配置
  * @factory: Server工厂
  * @return 如果创建失败返回NULL，否则返回非NULL
  */
extern server_t create(server::IConfig* config, server::IFactory* factory);

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_H
