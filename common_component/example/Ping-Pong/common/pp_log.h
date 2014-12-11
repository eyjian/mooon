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
 * Author: laocai_liu@qq.com or laocailiu@gmail.com
 */

#ifndef MOOON_PP_LOG_H
#define MOOON_PP_LOG_H

#include <sys/log.h>
#include "config.h"

PP_NAMESPACE_BEGIN

#define PP_MODULE_NAME "PINGPONG"

#define PP_LOG_BIN(log, size)         __MYLOG_BIN(pingpong::pp_logger, log, size)
#define PP_LOG_TRACE(format, ...)     __MYLOG_TRACE(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)
#define PP_LOG_FATAL(format, ...)     __MYLOG_FATAL(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)
#define PP_LOG_ERROR(format, ...)     __MYLOG_ERROR(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)
#define PP_LOG_WARN(format, ...)      __MYLOG_WARN(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)
#define PP_LOG_INFO(format, ...)      __MYLOG_INFO(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)
#define PP_LOG_DEBUG(format, ...)     __MYLOG_DEBUG(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)
#define PP_LOG_DETAIL(format, ...)    __MYLOG_DETAIL(pingpong::pp_logger, PP_MODULE_NAME, format, ##__VA_ARGS__)

extern sys::ILogger* pp_logger;

PP_NAMESPACE_END

#endif // MOOON_PP_LOG_H

