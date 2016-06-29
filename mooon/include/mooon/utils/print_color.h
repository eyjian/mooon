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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_UTILS_PRINT_COLOR_H
#define MOOON_UTILS_PRINT_COLOR_H
#include <mooon/utils/config.h>

// 示例：
// printf(PRINT_COLOR_YELLOW"[WARN][%s:%d]"PRINT_COLOR_NONE, __FILE__, __LINE__);

#define PRINT_COLOR_NONE         "\033[m"
#define PRINT_COLOR_RED          "\033[0;32;31m"
#define PRINT_COLOR_YELLOW       "\033[1;33m"
#define PRINT_COLOR_BLUE         "\033[0;32;34m"
#define PRINT_COLOR_GREEN        "\033[0;32;32m"
#define PRINT_COLOR_WHITE        "\033[1;37m"
#define PRINT_COLOR_CYAN         "\033[0;36m"      // 蓝绿色, 青色
#define PRINT_COLOR_PURPLE       "\033[0;35m"      // 紫色
#define PRINT_COLOR_BROWN        "\033[0;33m"      // 褐色, 棕色
#define PRINT_COLOR_DARY_GRAY    "\033[1;30m"      // 卡里灰色
#define PRINT_COLOR_LIGHT_RED    "\033[1;31m"
#define PRINT_COLOR_LIGHT_GREEN  "\033[1;32m"
#define PRINT_COLOR_LIGHT_BLUE   "\033[1;34m"
#define PRINT_COLOR_LIGHT_CYAN   "\033[1;36m"
#define PRINT_COLOR_LIGHT_PURPLE "\033[1;35m"
#define PRINT_COLOR_LIGHT_GRAY   "\033[0;37m"      // 亮灰色

#endif // MOOON_UTILS_PRINT_COLOR_H
