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
#include "sys/error.h"
#include "utils/string_utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
SYS_NAMESPACE_BEGIN

void CDirUtils::list(const std::string& dirpath
                  , std::vector<std::string>* subdir_names
                  , std::vector<std::string>* file_names
                  , std::vector<std::string>* link_names) throw (CSyscallException)
{
    int errcode = 0;
    DIR* dir = opendir(dirpath.c_str());
    if (NULL == dir)
    {
        errcode = errno;
        THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("open `%s` error: %s", dirpath.c_str(), Error::to_string(errcode).c_str()), errno, "opendir");
    }

    for (;;)
    {
        errno = 0;

        const struct dirent* ent = readdir(dir);
        if (NULL == ent)
        {
            if (errno != 0)
            {
                errcode = errno;
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

bool CDirUtils::exist(const std::string& dirpath) throw (CSyscallException)
{
    struct stat buf;

    if (-1 == stat(dirpath.c_str(), &buf))
    {
        const int errcode = errno;
        if (ENOENT == errcode) // A component of the path path does not exist, or the path is an empty string.
            return false;
        if (ENOTDIR == errcode) // A component of the path is not a directory.
            return false;

        THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("stat `%s` error: %s", dirpath.c_str(), Error::to_string(errcode).c_str()), errcode, "stat");
    }

    return S_ISDIR(buf.st_mode);
}

void CDirUtils::create_directory(const char* dirpath, mode_t permissions)
{
    if (-1 == mkdir(dirpath, permissions))
    {
        const int errcode = errno;
        if (errcode != EEXIST)
        {
            THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("mkdir `%s` error: %s", dirpath, Error::to_string(errcode).c_str()), errcode, "mkdir");
        }
    }
}

void CDirUtils::create_directory_recursive(const char* dirpath, mode_t permissions)
{
    char* slash;
    char* pathname = strdupa(dirpath); // _GNU_SOURCE
    char* pathname_p = pathname;
    int errcode = 0;

    // 过滤掉头部的斜杠
    while ('/' == *pathname_p) ++pathname_p;

    for (;;)
    {
        slash = strchr(pathname_p, '/');
        if (NULL == slash) // 叶子目录
        {
            if (0 == mkdir(pathname, permissions)) break;
            if (EEXIST == errno) break;

            errcode = errno;
            THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("mkdir `%s` error: %s", pathname, Error::to_string(errcode).c_str()), errcode, "mkdir");
        }

        *slash = '\0';
        if ((-1 == mkdir(pathname, permissions)) && (errno != EEXIST))
        {
            errcode = errno;
            THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("mkdir `%s` error: %s", pathname, Error::to_string(errcode).c_str()), errcode, "mkdir");
        }

        *slash++ = '/';
        while ('/' == *slash) ++slash; // 过滤掉相连的斜杠
        pathname_p = slash;
    }
}

void CDirUtils::create_directory_byfilepath(const char* filepath, mode_t permissions)
{
    std::string dirpath = utils::CStringUtils::extract_dirpath(filepath);
    create_directory_recursive(dirpath.c_str(),  permissions);
}

SYS_NAMESPACE_END
