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
#include "sys/dir_utils.h"
SYS_NAMESPACE_BEGIN

void CDirUtils::list(const std::string& dirpath
                  , std::vector<std::string>* subdir_names
                  , std::vector<std::string>* file_names
                  , std::vector<std::string>* link_names) throw (CSyscallException)
{
    DIR* dir = opendir(dirpath.c_str());
    if (NULL == dir)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "opendir");

    for (;;)
    {
        errno = 0;
        struct dirent* ent = readdir(dir);
        if (NULL == ent)
        {
            if (errno != 0)
            {
                int errcode = errno;
                if (EACCES == errcode)
                {
                    // 忽略无权限的
                    continue;
                }

                closedir(dir);
                THROW_SYSCALL_EXCEPTION(NULL, errcode, "readdir");
            }

            break; // over
        }

        // 排除当前目录和父目录
        if ((0 == strcmp(ent->d_name, "."))
         || (0 == strcmp(ent->d_name, "..")))
        {
            continue;
        }

        if (DT_DIR == ent->d_type)
        {
            if (subdir_names != NULL)
                subdir_names->push_back(ent->d_name);
        }
        else if (DT_REG == ent->d_type)
        {
            if (file_names != NULL)
                file_names->push_back(ent->d_name);
        }
        else if (DT_LNK == ent->d_type)
        {
            if (link_names != NULL)
                link_names->push_back(ent->d_name);
        }
    }

    closedir(dir);
}

void CDirUtils::remove(const std::string& dirpath) throw (CSyscallException)
{
    if (-1 == rmdir(dirpath.c_str()))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "rmdir");
}

SYS_NAMESPACE_END
