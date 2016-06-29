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
#ifndef MOOON_OBSERVER_LOG_H
#define MOOON_OBSERVER_LOG_H
#include "mooon/observer/observer_manager.h"
OBSERVER_NAMESPACE_BEGIN

#define OBSERVER_MODULE_NAME "OBSERVER"

#define OBSERVER_LOG_BIN(log, size)         __MYLOG_BIN(observer::logger, log, size)
#define OBSERVER_LOG_TRACE(format, ...)     __MYLOG_TRACE(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define OBSERVER_LOG_FATAL(format, ...)     __MYLOG_FATAL(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define OBSERVER_LOG_ERROR(format, ...)     __MYLOG_ERROR(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define OBSERVER_LOG_WARN(format, ...)      __MYLOG_WARN(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define OBSERVER_LOG_INFO(format, ...)      __MYLOG_INFO(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define OBSERVER_LOG_DEBUG(format, ...)     __MYLOG_DEBUG(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)
#define OBSERVER_LOG_DETAIL(format, ...)    __MYLOG_DETAIL(observer::logger, OBSERVER_MODULE_NAME, format, ##__VA_ARGS__)

OBSERVER_NAMESPACE_END
#endif // MOOON_OBSERVER_LOG_H
