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
#include <zlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sys/file_utils.h"
#include "sys/close_helper.h"
SYS_NAMESPACE_BEGIN

bool CFileUtils::exists(const char* filepath) throw (CSyscallException)
{
    if (0 == access(filepath, F_OK))
    {
        return true;
    }
    else
    {
        if (ENOENT == errno)
            return false;
        THROW_SYSCALL_EXCEPTION(NULL, errno, "access");
    }
}

size_t CFileUtils::file_copy(int src_fd, int dst_fd) throw (CSyscallException)
{
    char buf[IO_BUFFER_MAX];
    size_t file_size = 0;

    for (;;)
    {    
        ssize_t ret = read(src_fd, buf, sizeof(buf)-1);
        if (ret > 0)
        {      
            for (;;)
            {            
                if (write(dst_fd, buf, ret) != ret)
                {
                    if (EINTR == errno) continue;
                    THROW_SYSCALL_EXCEPTION(NULL, errno, "write");
                }

                break;
            }

            file_size += (size_t)ret;
        }
        else if (0 == ret)
        {
            break;
        }
        else
        {
            if (EINTR == errno) continue;
            THROW_SYSCALL_EXCEPTION(NULL, errno, "read");
        }
    }

    return file_size;
}

size_t CFileUtils::file_copy(int src_fd, const char* dst_filename) throw (CSyscallException)
{    
    int dst_fd = open(dst_filename, O_WRONLY|O_CREAT|O_EXCL);
    if (-1 == src_fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");

    sys::CloseHelper<int> ch(dst_fd);
    return file_copy(src_fd, dst_fd);
}

size_t CFileUtils::file_copy(const char* src_filename, int dst_fd) throw (CSyscallException)
{
    int src_fd = open(src_filename, O_RDONLY);
    if (-1 == src_fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");

    sys::CloseHelper<int> ch(src_fd);
    return file_copy(src_fd, dst_fd);
}

size_t CFileUtils::file_copy(const char* src_filename, const char* dst_filename) throw (CSyscallException)
{ 
    int src_fd = open(src_filename, O_RDONLY);
    if (-1 == src_fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");

    sys::CloseHelper<int> src_ch(src_fd);
    int dst_fd = open(dst_filename, O_WRONLY|O_CREAT|O_EXCL);
    if (-1 == dst_fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");

    sys::CloseHelper<int> dst_ch(dst_fd);
    return file_copy(src_fd, dst_fd);
}

off_t CFileUtils::get_file_size(int fd) throw (CSyscallException)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    if (!S_ISREG(buf.st_mode))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    return buf.st_size;
}

off_t CFileUtils::get_file_size(const char* filepath) throw (CSyscallException)
{
    int fd = open(filepath, O_RDONLY);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    sys::CloseHelper<int> ch(fd);
    return get_file_size(fd);
}

uint32_t CFileUtils::crc32_file(int fd) throw (CSyscallException)
{
    uint32_t crc = 0;
    int page_size = sys::CUtils::get_page_size();
    char* buffer = new char[page_size];
    utils::DeleteHelper<char> dh(buffer, true);

    if (-1 == lseek(fd, 0, SEEK_SET))
    {
        THROW_SYSCALL_EXCEPTION(NULL, errno, "lseek");
    }
    for (;;)
    {
        int retval = read(fd, buffer, page_size);
        if (0 == retval)
        {
            break;
        }
        if (-1 == retval)
        {
            THROW_SYSCALL_EXCEPTION(NULL, errno, "read");
        }

        crc = crc32(crc, (unsigned char*)buffer, retval);
        if (retval < page_size)
        {
            break;
        }
    }
    if (-1 == lseek(fd, 0, SEEK_SET))
    {
        THROW_SYSCALL_EXCEPTION(NULL, errno, "lseek");
    }
    
    return crc;
}

uint32_t CFileUtils::crc32_file(const char* filepath) throw (CSyscallException)
{
    int fd = open(filepath, O_RDONLY);
    if (-1 == fd)
        THROW_SYSCALL_EXCEPTION(NULL, errno, "open");

    sys::CloseHelper<int> ch(fd);
    return crc32_file(fd);
}

uint32_t CFileUtils::get_file_mode(int fd) throw (CSyscallException)
{
    struct stat st;
    if (-1 == fstat(fd, &st))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    return st.st_mode;
}

void CFileUtils::remove(const char* filepath) throw (CSyscallException)
{
    if (-1 == unlink(filepath))
    {
        if (errno != ENOENT)
            THROW_SYSCALL_EXCEPTION(NULL, errno, "unlink");
    }
}

void CFileUtils::rename(const char* from_filepath, const char* to_filepath) throw (CSyscallException)
{
    if (-1 == ::rename(from_filepath, to_filepath))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "rename");
}

SYS_NAMESPACE_END
