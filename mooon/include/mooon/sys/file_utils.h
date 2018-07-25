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
#ifndef MOOON_SYS_FILE_UTILS_H
#define MOOON_SYS_FILE_UTILS_H
#include "mooon/sys/utils.h"
SYS_NAMESPACE_BEGIN

/**
  * 文件相关的工具类
  */
class CFileUtils
{
public:
    // 是否存在指定的文件
    static bool exists(const char* filepath) throw (CSyscallException);

    /** 文件复制函数
      * @src_fd: 打开的源文件句柄
      * @dst_fd: 打开的目的文件句柄
      * @return: 返回文件大小
      * @exception: 出错抛出CSyscallException异常
      */
    static size_t file_copy(int src_fd, int dst_fd) throw (CSyscallException);
    static size_t file_copy(int src_fd, const char* dst_filename) throw (CSyscallException);
    static size_t file_copy(const char* src_filename, int dst_fd) throw (CSyscallException);
    static size_t file_copy(const char* src_filename, const char* dst_filename) throw (CSyscallException);

    /** 得到文件字节数
      * @fd: 文件句柄
      * @return: 返回文件字节数
      * @exception: 出错抛出CSyscallException异常
      */
    static off_t get_file_size(int fd) throw (CSyscallException);
    static off_t get_file_size(const char* filepath) throw (CSyscallException);
    
    /***
      * 求得32位的文件CRC值
      * @exception: 出错抛出CSyscallException异常
      * @return 返回整个文件的CRC值
      * 注意：crc32_file会修改读写文件的偏移值
      */
    static uint32_t crc32_file(int fd) throw (CSyscallException);
    static uint32_t crc32_file(const char* filepath) throw (CSyscallException);
    
    /***
      * 获取文件权限模式
      * @exception: 出错抛出CSyscallException异常
      * @return 返回文件权限模式值
      */
    static uint32_t get_file_mode(int fd) throw (CSyscallException);

    /***
      * 删除一个文件
      * @filepath 需要删除的文件路径
      * @exception 如果出错，抛出sys::CSyscallException异常
      */
    static void remove(const char* filepath) throw (CSyscallException);

    /***
     * 重命名一个文件
     * from_filepath 被重命名的文件，可包含文件路径和文件名
     * to_filepath 新的文件，可包含文件路径和文件名
     */
    static void rename(const char* from_filepath, const char* to_filepath) throw (CSyscallException);
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_FILE_UTILS_H
