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
#ifndef MOOON_SERVER_LOG_H
#define MOOON_SERVER_LOG_H
#include "mooon/server/server.h"
SERVER_NAMESPACE_BEGIN
    
#define SERVER_MODULE_NAME "SERVER"

#define SERVER_LOG_BIN(log, size)         __MYLOG_BIN(server::logger, log, size)
#define SERVER_LOG_TRACE(format, ...)     __MYLOG_TRACE(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define SERVER_LOG_FATAL(format, ...)     __MYLOG_FATAL(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define SERVER_LOG_ERROR(format, ...)     __MYLOG_ERROR(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define SERVER_LOG_WARN(format, ...)      __MYLOG_WARN(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define SERVER_LOG_INFO(format, ...)      __MYLOG_INFO(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define SERVER_LOG_DEBUG(format, ...)     __MYLOG_DEBUG(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define SERVER_LOG_DETAIL(format, ...)    __MYLOG_DETAIL(server::logger, SERVER_MODULE_NAME, format, ##__VA_ARGS__)

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_LOG_H
