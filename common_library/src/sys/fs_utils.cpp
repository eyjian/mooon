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
#include <fcntl.h>
#include <paths.h>
#include <fstab.h>
#include <mntent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "sys/fs_utils.h"
#include "sys/close_helper.h"

#if COMPILE_FS_UTIL_CPP==1 // 必须在sys/sys_config.h之后
SYS_NAMESPACE_BEGIN

// fstab.h: #define  _PATH_FSTAB    "/etc/fstab"
// paths.h: #define  _PATH_MOUNTED  "/etc/mtab"

void CFSUtils::stat_fs(int fd, fs_stat_t& stat_buf) throw (CSyscallException)
{
    struct statvfs buf;
    if (-1 == fstatvfs(fd, &buf)) // 不使用statfs
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstatvfs");

    stat_buf.block_bytes = buf.f_bsize;
    stat_buf.total_block_nubmer = buf.f_blocks;
    stat_buf.free_block_nubmer = buf.f_bfree;
    stat_buf.avail_block_nubmer = buf.f_bavail;
    stat_buf.total_file_node_nubmer = buf.f_files;
    stat_buf.free_file_node_nubmer = buf.f_ffree;
    stat_buf.avail_file_node_nubmer = buf.f_favail;
    stat_buf.file_name_length_max = buf.f_namemax;
}

void CFSUtils::stat_fs(const char* path, fs_stat_t& stat_buf) throw (CSyscallException)
{
    int fd = open(path, O_RDONLY);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");

    CloseHelper<int> ch(fd);
    stat_fs(fd, stat_buf);
}

//////////////////////////////////////////////////////////////////////////
// CFSTable

CFSTable::CFSTable(bool mounted, const char* fsname_prefix) throw ()
{
    if (mounted)
        _fp = setmntent(_PATH_MOUNTED, "r");
    else
        _fp = setmntent(_PATH_FSTAB, "r");

    if (fsname_prefix != NULL)
        _fsname_prefix = fsname_prefix;
}

CFSTable::~CFSTable() throw ()
{
    if (_fp != NULL)
    {        
        endmntent(_fp);
        _fp = NULL;
    }
}

void CFSTable::reset() throw ()
{
    if (_fp != NULL)
        rewind(_fp);
}

CFSTable::fs_entry_t* CFSTable::get_entry(fs_entry_t& entry) throw ()
{
    struct mntent* ent = NULL;

    for (;;)
    {    
        ent = getmntent(_fp);
        if (NULL == ent) return NULL; // 找完了所有的

        if (_fsname_prefix.empty())
        {
            break;
        }
        else
        {
            // 文件系统名的前经验不匹配，则找下一个
            if (0 == strncmp(ent->mnt_fsname, _fsname_prefix.c_str(), _fsname_prefix.length()))
                break; // 找到一个
        }
    }

    entry.fs_name   = ent->mnt_fsname;
    entry.dir_path  = ent->mnt_dir;
    entry.type_name = ent->mnt_type;
    return &entry;
}

SYS_NAMESPACE_END
#endif // COMPILE_FS_UTIL_CPP
