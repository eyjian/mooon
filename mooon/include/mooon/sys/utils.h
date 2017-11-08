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
#ifndef MOOON_SYS_UTILS_H
#define MOOON_SYS_UTILS_H
#include "mooon/sys/error.h"
#include "mooon/sys/syscall_exception.h"
#include <stdlib.h> // srand
#include <sys/time.h> // gettimeofday
#include <vector>
SYS_NAMESPACE_BEGIN

/***
  * 与系统调用有关的工具类函数实现
  */
class CUtils
{
public:
    /** 线程安全的毫秒级sleep函数
      * @milliseconds: 需要sleep的毫秒数
      */
    static void millisleep(uint32_t milliseconds);
    static void microsleep(uint32_t microseconds);

    /** 得到指定系统调用错误码的字符串错误信息
      * @errcode: 系统调用错误码
      * @return: 系统调用错误信息
      */
    static std::string get_error_message(int errcode);

    /** 得到最近一次的出错信息 */
    static std::string get_last_error_message();

    /** 得到最近一次的出错代码 */
    static int get_last_error_code();

    /** 得到当前进程号 */
    static uint32_t get_current_process_id();

    /** 得到当前用户ID */
    static uint32_t get_current_userid();

    /** 得到当前用户名 */
    static std::string get_current_username();

    /** 得到当前进程所属可执行文件所在的全路径，包括文件名部分 */
    static std::string get_program_fullpath();

    /** 得到当前进程所属可执行文件所在的绝对路径，不包括文件名部分，并且结尾符不含反斜杠 */
    static std::string get_program_dirpath();
    static std::string get_program_path(); // 新的代码请使用get_program_dirpath，不要使用get_program_path
    
    /**
     * 取得指定进程的命令行参数
     * pid 如果为0表示取当前进程的
     * parameters 存储命令行参数
     *
     * 返回命令行参数个数
     */
    static int get_program_parameters(std::vector<std::string>* parameters, uint32_t pid=0);

    /**
     * 取得指定进程的整个命令行
     * 假设运行命令：cat /proc/self/cmdline 1 2 3 4 5，
     * 那么返回结果将是：cat/proc/self/cmdline12345
     *
     * 如果pid值为0则表示取当前进程的
     * separator 指定各参数间的分隔字符
     */
    static std::string get_program_full_cmdline(char separator='|', uint32_t pid=0);

    /** 得到与指定fd相对应的文件名，包括路径部分
      * @fd: 文件描述符
      * @return: 文件名，包括路径部分，如果失败则返回空字符串
      */
	static std::string get_filename(int fd);

    /** 得到一个目录的绝对路径，路径中不会包含../和./等，是一个完整的原始路径
      * @directory: 目录的相对或绝对路径
      * @return: 返回目录的绝对路径，如果出错则返回空字符串
      */
    static std::string get_full_directory(const char* directory);

    /** 得到CPU核个数
      * @return: 如果成功，返回大于0的CPU核个数，否则返回0
      */
	static uint16_t get_cpu_number();    

    /** 得到当前调用栈
      * 注意事项: 编译源代码时带上-rdynamic和-g选项，否则可能看到的是函数地址，而不是函数符号名称
      * @call_stack: 存储调用栈
      * @return: 成功返回true，否则返回false
      */
    static bool get_backtrace(std::string& call_stack);

    /** 得到指定目录字节数大小，非线程安全函数，同一时刻只能被一个线程调用
      * @dirpath: 目录路径
      * @return: 目录字节数大小
      */
    static off_t du(const char* dirpath);

    /** 得到内存页大小 */
    static int get_page_size();

    /** 得到一个进程可持有的最多文件(包括套接字等)句柄数 */
    static int get_fd_max();
    
    /** 下列is_xxx函数如果发生错误，则抛出CSyscallException异常 */
    static bool is_file(int fd);                 /** 判断指定fd对应的是否为文件 */
    static bool is_file(const char* path);       /** 判断指定Path是否为一个文件 */
    static bool is_link(int fd);                 /** 判断指定fd对应的是否为软链接 */
    static bool is_link(const char* path);       /** 判断指定Path是否为一个软链接 */
    static bool is_directory(int fd);            /** 判断指定fd对应的是否为目录 */
    static bool is_directory(const char* path);  /** 判断指定Path是否为一个目录 */
    
    /***
      * 是否允许当前进程生成coredump文件
      * @enable: 如果为true，则允许当前进程生成coredump文件，否则禁止
      * @core_file_size: 允许生成的coredump文件大小，如果取值小于0，则表示不限制文件大小
      * @exception: 如果调用出错，则抛出CSyscallException异常
      */
    static void enable_core_dump(bool enabled=true, int core_file_size=-1);

    /** 得到当前进程程序的名称，结果和main函数的argv[0]相同，如“./abc.exe”为“./abc.exe” */
    static std::string get_program_long_name();

    /**
     * 得到当前进程的的名称，不包含目录部分，如“./abc.exe”值为“abc.exe”
     * 如果调用了set_process_title()，
     * 则通过program_invocation_short_name可能取不到预期的值，甚至返回的是空
     */
    static std::string get_program_short_name();

    /**
     * 取路径的文件名部分，结果包含后缀部分，效果如下：
     *
     * path           dirpath        basename
     * "/usr/lib"     "/usr"         "lib"
     * "/usr/"        "/"            "usr"
     * "usr"          "."            "usr"
     * "/"            "/"            "/"
     * "."            "."            "."
     * ".."           "."            ".."
     */
    static std::string get_filename(const std::string& filepath);

    /** 取路径的目录部分，不包含文件名部分，并保证不以反斜杠结尾 */
    static std::string get_dirpath(const std::string& filepath);

    /***
      * 设置线程的名称
      * @new_name 新的名字，通过ps或top查看线程时，可以看到线程的名字
      * @exception: 如果调用出错，则抛出CSyscallException异常
      */
    static void set_process_name(const std::string& new_name);
    static void set_process_name(const char* format, ...);

    /***
      * 设置进程标题，ps命令看到的结果，
      * 必须先调用init_program_title()后，才可以调用set_program_title()
      */
    static void init_process_title(int argc, char *argv[]);    
    static void set_process_title(const std::string& new_title);
    static void set_process_title(const char* format, ...);

    /***
	  * 通用的pipe读取操作
	  * 读取方法为：先读一个4字节的长度，然后根据长度读取内容
	  * @fd pipe的句柄
	  * @buffer 存储从pipe中读取的数据，注意调用者使用后必须调用delete []buffer以释放内存
	  * @buffer_size 存储从pipe中读取到的数据字节数
	  * @exception: 如果调用出错，则抛出CSyscallException异常
	  */
    static void common_pipe_read(int fd, char** buffer, int32_t* buffer_size);

    /***
	  * 通用的pipe写操作
	  * 读取方法为：先写一个4字节的长度buffer_size，然后根据长度buffer_size写入内容
	  * @fd pipe的句柄
	  * @buffer 需要写入pipe的内容
	  * @buffer_size 需要写入的字节数
	  * @exception: 如果调用出错，则抛出CSyscallException异常
	  */
    static void common_pipe_write(int fd, const char* buffer, int32_t buffer_size);

    // 取随机数
    template <typename T>
    static T get_random_number(unsigned int i, T max_number)
    {
        struct timeval tv;
        struct timezone *tz = NULL;

        gettimeofday(&tv, tz);
        srandom(tv.tv_usec + i); // 加入i，以解决过快时tv_usec值相同

        // RAND_MAX 类似于INT_MAX
        return static_cast<T>(random() % max_number);
    }

    // 将一个vector随机化
    template <typename T>
    static void randomize_vector(std::vector<T>& vec)
    {
        unsigned int j = 0;
        std::vector<T> tmp;

        while (!vec.empty())
        {
            typename std::vector<T>::size_type max_size = vec.size();
            typename std::vector<T>::size_type random_number = mooon::sys::CUtils::get_random_number(j, max_size);

            typename std::vector<T>::iterator iter = vec.begin() + random_number;
            tmp.push_back(*iter);
            vec.erase(iter);
            ++j;
        }

        vec.swap(tmp);
    }

    // 取一个随机字符串
    static std::string get_random_string();
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_UTILS_H
