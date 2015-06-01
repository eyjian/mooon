// Writed by yijian, eyjian@qq.com or eyjian@gmail.com
#ifndef MOOON_SYS_DIR_UTILS_H
#define MOOON_SYS_DIR_UTILS_H
#include "mooon/sys/utils.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
SYS_NAMESPACE_BEGIN

class CDirUtils
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
                   , std::vector<std::string>* link_names=NULL) throw (CSyscallException);

    /***
      * 删除一个空目录
      * @exception 如果发生错误，则抛出sys::CSyscallException异常
      */
    static void remove(const std::string& dirpath) throw (CSyscallException);

    // 判断是否存在指定的目录
    static bool exist(const std::string& dirpath) throw (CSyscallException);
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_DIR_UTILS_H
