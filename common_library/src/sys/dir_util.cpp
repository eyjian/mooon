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
#include "sys/dir_util.h"
SYS_NAMESPACE_BEGIN

void CDirUtil::list(const std::string& dirpath
                  , std::vector<std::string>* subdir_names
                  , std::vector<std::string>* file_names
                  , std::vector<std::string>* link_names)
{
    DIR* dir = opendir(dirpath.c_str());
    if (NULL == dir)
        throw CSyscallException(errno, __FILE__, __LINE__, "opendir");

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
                    // Èç¹ûÊÇÈ¨ÏÞÎÊÌâ£¬ÔòºöÂÔ¼ÌÐø
                    continue;
                }
                
                closedir(dir);
                throw CSyscallException(errcode, __FILE__, __LINE__, "readdir");
            }

            break; // over
        }

        // ¹ýÂËµôµ±Ç°Ä¿Â¼ºÍËüµÄ¸¸Ä¿Â¼
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

void CDirUtil::remove(const std::string& dirpath)
{
    if (-1 == rmdir(dirpath.c_str()))
        throw CSyscallException(errno, __FILE__, __LINE__, "rmdir");
}

SYS_NAMESPACE_END
