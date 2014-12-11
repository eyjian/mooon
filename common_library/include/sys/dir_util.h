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
#ifndef MOOON_SYS_DIR_UTIL_H
#define MOOON_SYS_DIR_UTIL_H
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "sys/util.h"
SYS_NAMESPACE_BEGIN

class CDirUtil
{
public:
    /***
      * 不递归地列出目录下的文件或目录
      * @dirpath 目录路径
      * @dirs 用来存储dirpath下的子目录名，如果为NULL，表示略过
      * @files 用来存储dirpath下的文件名，如果为NULL，表示略过
      * @links 用来存储dirpath下的链接名，如果为NULL，表示略过
      * @exception 如果发生错误，则抛出sys::CSyscallException异常
      */
    static void list(const std::string& dirpath
                   , std::vector<std::string>* subdir_names
                   , std::vector<std::string>* file_names
                   , std::vector<std::string>* link_names=NULL);

    /***
      * 删除一个空目录
      * @exception 如果发生错误，则抛出sys::CSyscallException异常
      */
    static void remove(const std::string& dirpath);
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_DIR_UTIL_H
