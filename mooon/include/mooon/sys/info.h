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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_SYS_INFO_H
#define MOOON_SYS_INFO_H
#include <vector>
#include "mooon/sys/config.h"
SYS_NAMESPACE_BEGIN

/***
  * 用来获取系统、内核和进程的各类实时信息，如CPU和内存数据
  *
  * 硬件给内核提供一个系统定时器用以计算和管理时间，
  * 内核通过编程预设系统定时器的频率，即节拍率（tick rate），
  * 每一个周期称作一个tick（节拍），Linux内核从2.5版内核开始把频率从100调高到1000，
  * 如果为1000即系统时钟中断执行粒度为1ms，意味着系统中周期事情最快为1ms执行一次，
  * 而不可能有更高的精度，具体100还是1000由宏HZ决定。
  * tick也是HZ的倒数，意即timer interrupt每发生一次中断的时间，如HZ为250时则tick为4毫秒（millisecond）。
  *
  * Linux用全局变量jiffies用来记录自系统启动以来产生的节拍的总数，
  * 启动时内核将该变量初始化为0，此后每次时钟中断处理程序都会增加该变量的值。
  * 一秒内时钟中断的次数等于Hz，所以jiffies一秒内增加的值也就是Hz。
  * 系统运行时间以秒为单位，等于jiffies/Hz。
  *
  * Linux内核使用四个宏解决jiffies溢出：time_after(a,b)，time_before(a,b) ，time_after_eq(a,b)，time_before_eq(a,b)
  * 注意，jiffies类型为无符号长整型(unsigned long)，其他任何类型存放它都不正确。
  *
  * 将以秒为单位的时间转化为jiffies：seconds * Hz
  * 将jiffies转化为以秒为单位的时间：jiffies / Hz
  *
  * jiffies定义于文件<linux/Jiffies.h>中，
  * HZ值定义在param.h，如：#define HZ 100
  *
  * 除系统定时器外，还有一个硬件时钟：实时时钟（RTC），用来持久存放系统时间，
  * 系统关闭后靠主板上的微型电池保持计时，相应的全局变量为timeval类型的xtime。
  */
class CInfo
{
public:
    /***
      * 系统当前实时信息
      */
    typedef struct
    {
        long uptime_second;             /* Seconds since boot */
        unsigned long average_load[3];  /* 1, 5, and 15 minute load averages */
        unsigned long ram_total;        /* Total usable main memory size */
        unsigned long ram_free;         /* Available memory size */
        unsigned long ram_shared;       /* Amount of shared memory */
        unsigned long ram_buffer;       /* Memory used by buffers */
        unsigned long swap_total;       /* Total swap space size */
        unsigned long swap_free;        /* swap space still available */
        unsigned short process_number;  /* Number of current processes */
    }sys_info_t;

    /***
      * 当前进程时间信息
      */
    typedef struct
    {
        long user_time;             /* user time */
        long system_time;          /* system time */
        long user_time_children;    /* user time of children */
        long system_time_children; /* system time of children */
    }process_time_t;

    /***
      * 当前系统CPU信息
      */
    typedef struct
    {
        // 单位: jiffies, 1jiffies=0.01秒
        uint64_t total;
        uint32_t user;    /** 从系统启动开始累计到当前时刻，处于用户态的运行时间，不包含 nice值为负进程 */
        uint32_t nice;    /** 从系统启动开始累计到当前时刻，nice值为负的进程所占用的CPU时间 */
        uint32_t system;  /** 从系统启动开始累计到当前时刻，处于核心态的运行时间 */
        uint32_t idle;    /** 从系统启动开始累计到当前时刻，除IO等待时间以外的其它等待时间 */
        uint32_t iowait;  /** 从系统启动开始累计到当前时刻，IO等待时间(2.5.41) */
        uint32_t irq;     /** 从系统启动开始累计到当前时刻，硬中断时间(2.6.0) */
        uint32_t softirq; /** 从系统启动开始累计到当前时刻，软中断时间(2.6.0) */
        //uint32_t stealstolen; /** which is the time spent in other operating systems when running in a virtualized environment(2.6.11) */
        //uint32_t guest;       /** which is the time spent running a virtual  CPU  for  guest operating systems under the control of the Linux kernel(2.6.24) */
    }cpu_info_t;

    /***
      * 当前系统内存信息
      */
    typedef struct
    {
        uint32_t mem_total;
        uint32_t mem_free;
        uint32_t buffers;
        uint32_t cached;
        uint32_t swap_cached;
        uint32_t swap_total;
        uint32_t swap_free;
    }mem_info_t;

    /***
      * 内核版本号
      */
    typedef struct
    {
        int16_t major;    /** 主版本号 */
        int16_t minor;    /** 次版本号(如果次版本号是偶数，那么内核是稳定版；若是奇数则是开发版) */
        int16_t revision; /** 修订版本号 */
    }kernel_version_t;

    /***
      * 当时进程状态信息
      *
      * 进程的状态值:
        D    Uninterruptible sleep (usually IO)
        R    Running or runnable (on run queue)
        S    Interruptible sleep (waiting for an event to complete)
        T    Stopped, either by a job control signal or because it is being traced.
        W    paging (not valid since the 2.6.xx kernel)
        X    dead (should never be seen)
        Z    Defunct ("zombie") process, terminated but not reaped by its parent.
      */
    typedef struct
    {
        /** 01 */ pid_t pid;                     /** 进程号，其允许的最大值，请查看/proc/sys/kernel/pid_max */
        /** 02 */ char comm[FILENAME_MAX];       /** 进程的名字，不包括路径 */
        /** 03 */ char state;                    /** 进程的状态 */
        /** 04 */ pid_t ppid;                    /** 父进程号 */
        /** 05 */ pid_t pgrp;                    /** 进程组号 */
        /** 06 */ pid_t session;                 /** 进程会话号 */
        /** 07 */ int tty_nr;                    /** tty终端的设备号（The tty the process uses） */
        /** 08 */ pid_t tpgid;                   /** 终端的进程组号（The tty the process uses） */
        /** 09 */ unsigned int flags;            /** 进程标志位（The kernel flags word of the process (%lu before Linux 2.6.22)） */
        /** 10 */ unsigned long minflt;          /** 不需要从硬盘拷数据而发生的缺页的次数（The number of minor faults the process has made which have not required loading a memory page from disk） */
        /** 11 */ unsigned long cminflt;         /** 累计的该任务的所有的waited-for进程曾经发生的次缺页的次数目（The number of minor faults that the process's waited-for children have made） */
        /** 12 */ unsigned long majflt;          /** 该任务需要从硬盘拷数据而发生的缺页（主缺页）的次数（The number of major faults the process has made which have required loading a memory page from disk） */
        /** 13 */ unsigned long cmajflt;         /** 累计的该任务的所有的waited-for进程曾经发生的主缺页的次数目（The number of major faults that the process's waited-for children have made） */
        /** 14 */ unsigned long utime;           /** 用户态运行的时间，单位为jiffies（The number of jiffies that this process has been scheduled in user mode） */
        /** 15 */ unsigned long stime;           /** 核心态运行的时间，单位为jiffies（The number of jiffies that this process has been scheduled in kernel mode） */
        /** 16 */ long cutime;                   /** 累计的该进程的所有的waited-for进程曾经在用户态运行的时间，单位为jiffies（The number of jiffies that this process's waited-for children have been scheduled in user mode） */
        /** 17 */ long cstime;                   /** 累计的该进程的所有的waited-for进程曾经在核心态运行的时间，单位为jiffies（The number of jiffies that this process's waited-for children have been scheduled in kernel mode） */
        /** 18 */ long priority;                 /** 动态优先级（The standard nice value, plus fifteen. The value is never negative in the kernel） */
        /** 19 */ long nice;                     /** 静态优先级（The nice value ranges from 19 (nicest) to -19 (not nice to others)） */
        /** 20 */ long num_threads;              /** 线程数（Number of threads in this process (since Linux 2.6).  Before kernel 2.6, this field was hard coded to 0 as a placeholder） */
        /** 21 */ long itrealvalue;              /** 由于计时间隔导致的下一个 SIGALRM 发送进程的时延，以jiffies为单位（The time in jiffies before the next SIGALRM is sent to the process due to an interval timer.2.6.17, this field is no longer maintained, and is hard coded as 0） */
        /** 22 */ long long starttime;           /** 启动的时间，单位为jiffies（The time in jiffies the process started after system boot） */
        /** 23 */ unsigned long vsize;           /** 当前占用的虚拟内存大小，以字节为单位（Virtual memory size in bytes） */
        /** 24 */ long rss;                      /** 当前占用的物理内存大小，以页为单位（Resident Set Size: number of pages the process has in real memory, minus 3 for administrative purposes） */
        /** 25 */ unsigned long rlim;            /** 能驻留物理地址空间的最大值，以字节为单位（Current limit in bytes on the rss of the process (usually 4294967295 on i386)） */
        /** 26 */ unsigned long startcode;       /** 在虚拟地址空间的代码段的起始地址（The address above which program text can run） */
        /** 27 */ unsigned long endcode;         /** 在虚拟地址空间的代码段的结束地址The address below which program text can run） */
        /** 28 */ unsigned long startstack;      /** 在虚拟地址空间的栈的开始地址（The address of the start of the stack） */
        /** 29 */ unsigned long kstkesp;         /** ESP当前值（堆栈指针）（The current value of esp (stack pointer), as found in the kernel stack page for the process） */
        /** 30 */ unsigned long kstkeip;         /** 指向将要执行的指令的指针（The current EIP (instruction pointer)） */
        /** 31 */ unsigned long signal;          /** 待处理信号的位图（ The bitmap of pending signals） */
        /** 32 */ unsigned long blocked;         /** 被阻塞信号的位图（The bitmap of blocked signals） */
        /** 33 */ unsigned long sigignore;       /** 忽略的信号的位图（The bitmap of ignored signals） */
        /** 34 */ unsigned long sigcatch;        /** 被俘获的信号的位图（The bitmap of caught signals） */
        /** 35 */ unsigned long nswap;           /** 被swapped的页数，当前未用（Number of pages swapped (not maintained)） */
        /** 36 */ unsigned long cnswap;          /** 所有子进程被swapped的页数的和，当前未用（Cumulative nswap for child processes (not maintained)） */
        /** 37 */ int exit_signal;               /** 结束时向父进程所发送的信号（Signal to be sent to parent when we die (since Linux 2.1.22)） */
        /** 38 */ int processor;                 /** 运行在哪个CPU上（CPU number last executed on (since Linux 2.2.8)） */
    }process_info_t;

    /***
      * 网卡流量数据结构
      */
    typedef struct
    {
        /** 01 */ char interface_name[INTERFACE_NAME_MAX]; /** 网卡名，如eth0 */

                  /** 接收数据 */
        /** 02 */ unsigned long receive_bytes;             /** 此网卡接收到的字节数 */
        /** 03 */ unsigned long receive_packets;
        /** 04 */ unsigned long receive_errors;
        /** 05 */ unsigned long receive_dropped;
        /** 06 */ unsigned long receive_fifo_errors;
        /** 07 */ unsigned long receive_frame;
        /** 08 */ unsigned long receive_compressed;
        /** 09 */ unsigned long receive_multicast;

                  /** 发送数据 */
        /** 10 */ unsigned long transmit_bytes;             /** 此网卡已发送的字节数 */
        /** 11 */ unsigned long transmit_packets;
        /** 12 */ unsigned long transmit_errors;
        /** 13 */ unsigned long transmit_dropped;
        /** 14 */ unsigned long transmit_fifo_errors;
        /** 15 */ unsigned long transmit_collisions;
        /** 16 */ unsigned long transmit_carrier;
        /** 17 */ unsigned long transmit_compressed;        
    }net_info_t;

    /***
      * 进程页信息结构
      * 一页大小通常为4k，
      * 具体可以调用mooon::sys::CUtils::get_page_size()取得
      */
    typedef struct
    {
        long size;     /** 程序大小 */
        long resident; /** 常驻内存空间大小 */
        long share;    /** 共享内存页数 */
        long text;     /** 代码段占用内存页数 */
        long lib;      /** 数据/堆栈段占用内存页数 */
        long data;     /** 引用库占用内存页数 */
    }process_page_info_t;

public:
    /** 获取系统信息，具体请参考sys_info_t的描述 */
    static bool get_sys_info(sys_info_t& sys_info);

    /** 获取内存信息，具体请参考mem_info_t的描述 */
    static bool get_mem_info(mem_info_t& mem_info);

    /** 获取总CPU信息，具体请参考cpu_info_t的描述 */
    static bool get_cpu_info(cpu_info_t& cpu_info);

    /** 获取所有CPU信息，具体请参考cpu_info_t的描述 */
    static int get_cpu_info_array(std::vector<cpu_info_t>& cpu_info_array);

    /** 得到内核版本号 */
    static bool get_kernel_version(kernel_version_t& kernel_version);

    /** 获取进程信息，具体请参考process_info_t的描述 */
    static bool get_process_info(process_info_t& process_info, pid_t pid);
    static bool get_process_info(process_info_t& process_info);

    /** 获取进程页信息，具体请参考process_page_info_t的描述 */
    static bool get_process_page_info(process_page_info_t& process_page_info, pid_t pid);
    static bool get_process_page_info(process_page_info_t& process_page_info);

    /** 获取进程运行时间数据，具体请参考process_time_t的描述 */
    static bool get_process_times(process_time_t& process_time);
        
    /***
      * 获取网卡流量等信息
      * 流量 = (当前获取的值 - 上一时间获取的值) / 两次间隔的时长
      * @interface_name: 网卡名，如eth0等
      * @net_info: 存储网卡流量等数据
      */
    static bool get_net_info(const char* interface_name, net_info_t& net_info);
    static bool get_net_info_array(std::vector<net_info_t>& net_info_array);    

private:
    static bool do_get_net_info_array(const char* interface_name, std::vector<net_info_t>& net_info_array);
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_INFO_H
