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

    /***
      * 递归的创建目录
      * @dirpath: 需要创建的目录
      * @permissions: 目录权限，取值可以为下列的任意组合:
      *                    S_IRWXU, S_IRUSR, S_IWUSR, S_IXUSR
      *                    S_IRWXG, S_IRGRP, S_IWGRP, S_IXGRP
      *                    S_IRWXO, S_IROTH, S_IWOTH, S_IXOTH
      * @exception: 出错则抛出CSyscallException
      */
    static void create_directory(const char* dirpath, mode_t permissions=DIRECTORY_DEFAULT_PERM);

    /***
      * 递归的创建目录
      * @dirpath: 需要创建的目录
      * @permissions: 目录权限
      * @exception: 出错则抛出CSyscallException
      */
    static void create_directory_recursive(const char* dirpath, mode_t permissions=DIRECTORY_DEFAULT_PERM);

    /***
      * 根据文件路径，递归的创建目录
      * @dirpath: 文件路径
      * @permissions: 目录权限
      * @exception: 出错则抛出CSyscallException
      */
    static void create_directory_byfilepath(const char* filepath, mode_t permissions=DIRECTORY_DEFAULT_PERM);
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_DIR_UTILS_H
