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
#ifndef MOOON_SCHEDULER_LOG_H
#define MOOON_SCHEDULER_LOG_H
#include <sys/log.h>
#include <scheduler/error_code.h>

#define SCHEDULER_MODULE_NAME "SCHEDULER"

#define SCHEDULER_LOG_BIN(log, size)         __MYLOG_BIN(scheduler::logger, log, size)
#define SCHEDULER_LOG_TRACE(format, ...)     __MYLOG_TRACE(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)
#define SCHEDULER_LOG_FATAL(format, ...)     __MYLOG_FATAL(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)
#define SCHEDULER_LOG_ERROR(format, ...)     __MYLOG_ERROR(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)
#define SCHEDULER_LOG_WARN(format, ...)      __MYLOG_WARN(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)
#define SCHEDULER_LOG_INFO(format, ...)      __MYLOG_INFO(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)
#define SCHEDULER_LOG_DEBUG(format, ...)     __MYLOG_DEBUG(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)
#define SCHEDULER_LOG_DETAIL(format, ...)    __MYLOG_DETAIL(scheduler::logger, SCHEDULER_MODULE_NAME, format, ##__VA_ARGS__)

#endif // MOOON_SCHEDULER_LOG_H
