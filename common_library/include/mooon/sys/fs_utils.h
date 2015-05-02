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
#ifndef MOOON_SYS_FS_UTILS_H
#define MOOON_SYS_FS_UTILS_H
#include "mooon/sys/utils.h"

#if COMPILE_FS_UTIL_CPP==1 /** 必须在sys/sys_config.h之后 */
SYS_NAMESPACE_BEGIN

/***
  * 下面是df命令的输出：
  * 文件系统               1K-块       已用     可用   已用%   挂载点
  * /dev/usr              674820      728644   232508   34%   /usr/local
  * /dev/test           87846516    16217732  2086744   23%   /test
  *
  * fs_stat_t的block_bytes可能是1K，但也可能是1K的整数倍，如4K
  * fs_stat_t的avail_block_nubmer是可用的块个数
  * fs_entry_t的fs_name对应于/dev/usr等
  * fs_entry_t的dir_path对应于/usr/local等
  *
  * 某分区可用空间大小计算公式（单位：字节）：
  * fs_stat_t.block_bytes * fs_stat_t.avail_block_nubmer
  */
     
/***
  * 操作文件系统的工具类
  * 通过与CFSTable类的结合，可以得到各分区的总大小和剩余大小等数据
  */
class CFSUtils
{
public:    
    typedef struct
    {
        unsigned long block_bytes;            /** 每个块的字节数大小 */
        unsigned long total_block_nubmer;     /** 总的块数 */
        unsigned long free_block_nubmer;      /** 没用使用的块数 */
        unsigned long avail_block_nubmer;     /** 非root用户可用的块数 */
        unsigned long total_file_node_nubmer; /** 总的文件结点数 */
        unsigned long free_file_node_nubmer;  /** 没有使用的文件结点数 */
        unsigned long avail_file_node_nubmer; /** 非root用户可用的文件结点数 */
        unsigned long file_name_length_max;   /** 支持的最大文件名长度 */
    }fs_stat_t;

public:
    /***
      * 统计指定fd所指向的文件所在的文件系统，得到该文件系统的数据信息
      * @fd: 文件句柄
      * @stat_buf: 存储统计信息
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    static void stat_fs(int fd, fs_stat_t& stat_buf) throw (CSyscallException);

    /***
      * 统计指定路径所指向的文件所在的文件系统，得到该文件系统的数据信息
      * @path: 路径，为所在分区任意存在的路径即可
      * @stat_buf: 存储统计信息
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    static void stat_fs(const char* path, fs_stat_t& stat_buf) throw (CSyscallException);
};

/***
  * 文件系统表
  * 示例:
  * CFSTable fst;
  * if (fst) {
  *     fs_entry_t ent;
  *     while (get_entry(ent)) {
  *         printf("fs_name=%s\n", ent.fs_name);
  *         printf("dir_path=%s\n", ent.dir_path);
  *         printf("type_name=%s\n", ent.type_name);
  *     }
  * }
  */
class CFSTable
{
public:
    typedef struct
    {
        std::string fs_name;   /** 文件系统名 */
        std::string dir_path;  /** 文件系统所加载的目录路径 */
        std::string type_name; /** 文件系统类型名，如ext3等 */
    }fs_entry_t;

public:
    /***
      * 构造对象，并打开文件系统表。对象生成后，应当先判断对象是否可用，如：
      * CFSTable mt; if (mt) { }，这测试用if语句不能少，只有为真时才可以调用get_entry
      * @mounted: 是否只针对已经加载的文件系统
      * @fsname_prefix: 所关心的文件系统名前缀，只有匹配的才关心，如果为NULL，则表示所有的
      */
    CFSTable(bool mounted=true, const char* fsname_prefix=NULL) throw ();
    ~CFSTable() throw ();

    /** 复位，可重新调用get_entry获取文件系统列表 */
    void reset() throw ();

    /***
      * 从文件系统表中取一条记录
      * @return: 如果取到记录，则返回指针entry的指针，否则返回NULL，表示已经遍历完所有的
      */
    fs_entry_t* get_entry(fs_entry_t& entry) throw ();

    /***
      * 判断在构造函数中是否成功打开了文件系统表
      * @return: 如果已经打开，则返回true，否则返回false
      */
    operator bool() const throw () { return _fp != NULL; }

private:
    FILE* _fp;
    std::string _fsname_prefix;
};

SYS_NAMESPACE_END
#endif // COMPILE_FS_UTIL_CPP
#endif // MOOON_SYS_FS_UTILS_H
