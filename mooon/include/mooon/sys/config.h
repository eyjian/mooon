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
#ifndef MOOON_SYS_CONFIG_H
#define MOOON_SYS_CONFIG_H
#include <mooon/utils/config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// 编译控制宏
#define HAVE_UIO_H 0          /** 是否可以使用writev和readv */
#define COMPILE_FS_UTIL_CPP 1 /** 是否编译fs_util.cpp */
#define ENABLE_SET_LOG_THREAD_NAME 1 /** 是否设置日志线程名 */

// 定义名字空间宏
#define SYS_NAMESPACE_BEGIN namespace mooon { namespace sys {
#define SYS_NAMESPACE_END                   }}
#define SYS_NAMESPACE_USE using namespace mooon::sys;

#ifndef S_IRGRP
#define S_IRGRP (S_IRUSR >> 3)  /* Read by group.  */
#endif // S_IRGRP

#ifndef S_IXGRP
#define S_IXGRP (S_IXUSR >> 3)  /* Execute by group.  */
#endif // S_IXGRP

#ifndef S_IXOTH
#define S_IXOTH (S_IXGRP >> 3)  /* Execute by others.  */
#endif // S_IXOTH

/** 新创建文件的默认权限 */
#define FILE_DEFAULT_PERM (S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)
/** 新创建目录的默认权限 */
#define DIRECTORY_DEFAULT_PERM (S_IRWXU | S_IXGRP | S_IXOTH)
/** 新创建的IPC(包括shm和sem等)默认权限 */
#define IPC_DEFAULT_PERM  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

/** 网卡名最大字节长度 */
#define INTERFACE_NAME_MAX 20

#endif // MOOON_SYS_CONFIG_H
