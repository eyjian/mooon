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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_OBSERVER_CONFIG_H
#define MOOON_OBSERVER_CONFIG_H
#include <mooon/utils/config.h>

/***
  * 编译宏开关
  */
#define ENABLE_SET_OBSERVER_THREAD_NAME 1 /** 是否允许设置observer线程名 */

/***
  * observer模块名称空间名称定义
  */
#define OBSERVER_NAMESPACE_BEGIN namespace mooon { namespace observer {
#define OBSERVER_NAMESPACE_END                   }}
#define OBSERVER_NAMESPACE_USE using mooon::observer;

#endif // MOOON_OBSERVER_CONFIG_H
