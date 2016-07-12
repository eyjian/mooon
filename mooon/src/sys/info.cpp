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
#include <sys/times.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include "sys/info.h"
#include "sys/close_helper.h"
#include "utils/string_utils.h"
SYS_NAMESPACE_BEGIN

bool CInfo::get_sys_info(sys_info_t& sys_info)
{
    struct sysinfo info;
    if (-1 == sysinfo(&info)) return false;

    sys_info.uptime_second   = info.uptime;
    sys_info.average_load[0] = info.loads[0];
    sys_info.average_load[1] = info.loads[1];
    sys_info.average_load[2] = info.loads[2];
    sys_info.ram_total       = info.totalram;
    sys_info.ram_free        = info.freeram;
    sys_info.ram_shared      = info.sharedram;
    sys_info.ram_buffer      = info.bufferram;
    sys_info.swap_total      = info.totalswap;
    sys_info.swap_free       = info.freeswap;
    sys_info.process_number  = info.procs;
    return true;
}

// sysconf(_SC_PAGESIZE)
// sysconf(_SC_PHYS_PAGES);
// sysconf(_SC_AVPHYS_PAGES);
bool CInfo::get_mem_info(mem_info_t& mem_info)
{
    FILE* fp = fopen("/proc/meminfo", "r");
    if (NULL == fp) return false;
    sys::CloseHelper<FILE*> ch(fp);

    int i = 0;
    int value;
    char name[LINE_MAX];
    char line[LINE_MAX];
	int filed_number = 2;
	int member_number = 7;

    while (fgets(line, sizeof(line)-1, fp))
    {
        if (sscanf(line, "%s%u", name, &value) != filed_number)
            continue;

        if (0 == strcmp(name, "MemTotal:"))
        {
            ++i;
            mem_info.mem_total = value;
        }
        else if (0 == strcmp(name, "MemFree:"))
        {
            ++i;
            mem_info.mem_free = value;
        }
        else if (0 == strcmp(name, "Buffers:"))
        {
            ++i;
            mem_info.buffers = value;
        }
        else if (0 == strcmp(name, "Cached:"))
        {
            ++i;
            mem_info.cached = value;
        }
        else if (0 == strcmp(name, "SwapCached:"))
        {
            ++i;
            mem_info.swap_cached = value;
        }
        else if (0 == strcmp(name, "SwapTotal:"))
        {
            ++i;
            mem_info.swap_total = value;
        }
        else if (0 == strcmp(name, "SwapFree:"))
        {
            ++i;
            mem_info.swap_free = value;
        }

        if (i == member_number)
            break;
    }

    return (i == member_number);
}

bool CInfo::get_cpu_info(cpu_info_t& cpu_info)
{
    FILE* fp = fopen("/proc/stat", "r");
    if (NULL == fp) return false;
    sys::CloseHelper<FILE*> ch(fp);

    char name[LINE_MAX];
    char line[LINE_MAX];
    int filed_number = 8;

    while (fgets(line, sizeof(line)-1, fp))
    {
        if (sscanf(line, "%s%u%u%u%u%u%u%u", name, &cpu_info.user, &cpu_info.nice, &cpu_info.system, &cpu_info.idle, &cpu_info.iowait, &cpu_info.irq, &cpu_info.softirq) != filed_number)
            continue;
        
        if (0 == strcmp(name, "cpu"))
        {
            cpu_info.total = cpu_info.user + cpu_info.nice + cpu_info.system + cpu_info.idle + cpu_info.iowait + cpu_info.irq + cpu_info.softirq;
            break;
        }

        name[0] = '\0';        
    }

    return (name[0] != '\0');
}

int CInfo::get_cpu_info_array(std::vector<cpu_info_t>& cpu_info_array)
{
    cpu_info_array.clear();

    FILE* fp = fopen("/proc/stat", "r");
    if (NULL == fp) return 0;
    sys::CloseHelper<FILE*> ch(fp);

    char name[LINE_MAX];
    char line[LINE_MAX];
    int filed_number = 8;
    
    while (fgets(line, sizeof(line)-1, fp))
    {
        cpu_info_t cpu_info;
        if (sscanf(line, "%s%u%u%u%u%u%u%u", name, &cpu_info.user, &cpu_info.nice, &cpu_info.system, &cpu_info.idle, &cpu_info.iowait, &cpu_info.irq, &cpu_info.softirq) != filed_number)
            continue;

        if (strncmp(name, "cpu", 3) != 0)
            break;
        
        cpu_info.total = cpu_info.user + cpu_info.nice + cpu_info.system + cpu_info.idle + cpu_info.iowait + cpu_info.irq + cpu_info.softirq;
        cpu_info_array.push_back(cpu_info);
    }

    return cpu_info_array.size();
}

bool CInfo::get_kernel_version(kernel_version_t& kernel_version)
{
    FILE* fp = fopen("/proc/version", "r");
    if (NULL == fp) return false;
    sys::CloseHelper<FILE*> ch(fp);

    char line[LINE_MAX];
	char* linep = fgets(line, sizeof(line)-1, fp);

    if (NULL == linep) return false;

    char f1[LINE_MAX];
    char f2[LINE_MAX];
    char version[LINE_MAX];
    // Linux version 2.6.24-22-generic (buildd@crested)
    if (sscanf(line, "%s%s%s", f1, f2, version) != 3)
        return false;

    char* bar = strchr(version, '-');
    if (bar != NULL) *bar = '\0';

    char* dot1 = strchr(version, '.');
    if (NULL == dot1) return false;
    *dot1++ = '\0';
    if (!utils::CStringUtils::string2int16(version, kernel_version.major)) return false;

    char* dot2 = strchr(dot1, '.');
    if (NULL == dot2) return false;
    *dot2++ = '\0';
    if (!utils::CStringUtils::string2int16(dot1, kernel_version.minor)) return false;

    return utils::CStringUtils::string2int16(dot2, kernel_version.revision);
}

bool CInfo::get_process_info(process_info_t& process_info)
{
    pid_t pid = getpid();
    return get_process_info(process_info, pid);
}

bool CInfo::get_process_info(process_info_t& process_info, pid_t pid)
{
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "/proc/%u/stat", pid);

    FILE* fp = fopen(filename, "r");
    if (NULL == fp) return false;
    sys::CloseHelper<FILE*> ch(fp);

    char line[LINE_MAX];
    int filed_number = 38;
	char* linep = fgets(line, sizeof(line)-1, fp);

    if (NULL == linep) return false;

    return (sscanf(line, "%d%s%s%d%d"
                         "%d%d%d%u%lu"
                         "%lu%lu%lu%lu%lu"
                         "%ld%ld%ld%ld%ld"
                         "%ld%lld%lu%ld%lu"
                         "%lu%lu%lu%lu%lu"
                         "%lu%lu%lu%lu%lu"
                         "%lu%d%d"
              /** 01 */ ,&process_info.pid
              /** 02 */ , process_info.comm
              /** 03 */ ,&process_info.state
              /** 04 */ ,&process_info.ppid
              /** 05 */ ,&process_info.pgrp
              /** 06 */ ,&process_info.session
              /** 07 */ ,&process_info.tty_nr
              /** 08 */ ,&process_info.tpgid
              /** 09 */ ,&process_info.flags
              /** 10 */ ,&process_info.minflt
              /** 11 */ ,&process_info.cminflt
              /** 12 */ ,&process_info.majflt
              /** 13 */ ,&process_info.cmajflt
              /** 14 */ ,&process_info.utime
              /** 15 */ ,&process_info.stime
              /** 16 */ ,&process_info.cutime
              /** 17 */ ,&process_info.cstime
              /** 18 */ ,&process_info.priority
              /** 19 */ ,&process_info.nice
              /** 20 */ ,&process_info.num_threads
              /** 21 */ ,&process_info.itrealvalue
              /** 22 */ ,&process_info.starttime
              /** 23 */ ,&process_info.vsize
              /** 24 */ ,&process_info.rss
              /** 25 */ ,&process_info.rlim
              /** 26 */ ,&process_info.startcode
              /** 27 */ ,&process_info.endcode
              /** 28 */ ,&process_info.startstack
              /** 29 */ ,&process_info.kstkesp
              /** 30 */ ,&process_info.kstkeip
              /** 31 */ ,&process_info.signal
              /** 32 */ ,&process_info.blocked
              /** 33 */ ,&process_info.sigignore
              /** 34 */ ,&process_info.sigcatch
              /** 35 */ ,&process_info.nswap
              /** 36 */ ,&process_info.cnswap
              /** 37 */ ,&process_info.exit_signal
              /** 38 */ ,&process_info.processor) == filed_number);
}

bool CInfo::get_process_page_info(process_page_info_t& process_page_info)
{
    pid_t pid = getpid();
    return get_process_page_info(process_page_info, pid);
}

bool CInfo::get_process_page_info(process_page_info_t& process_page_info, pid_t pid)
{
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "/proc/%u/statm", pid);

    FILE* fp = fopen(filename, "r");
    if (NULL == fp) return false;
    sys::CloseHelper<FILE*> ch(fp);

    char line[LINE_MAX];
    int filed_number = 6;
	char* linep = fgets(line, sizeof(line)-1, fp);

    if (NULL == linep) return false;

    return (sscanf(line
                  ,"%ld%ld%ld%ld%ld%ld"
                  ,&process_page_info.size
                  ,&process_page_info.resident
                  ,&process_page_info.share
                  ,&process_page_info.text
                  ,&process_page_info.lib
                  ,&process_page_info.data) == filed_number);
}

// getrusage

bool CInfo::get_process_times(process_time_t& process_time)
{
    struct tms buf;
    if (-1 == times(&buf)) return false;
    
    process_time.user_time            = buf.tms_utime;
    process_time.system_time          = buf.tms_stime;
    process_time.user_time_children   = buf.tms_cutime;
    process_time.system_time_children = buf.tms_cstime;
    
    return true;
}

bool CInfo::do_get_net_info_array(const char* interface_name, std::vector<net_info_t>& net_info_array)
{
    net_info_array.clear();
    
    FILE* fp = fopen("/proc/net/dev", "r");
    if (NULL == fp) return false;
    sys::CloseHelper<FILE*> ch(fp);

    char line[LINE_MAX];
    int filed_number = 17;

    // 跳过头两行
    if (NULL == fgets(line, sizeof(line)-1, fp)) return false;
    if (NULL == fgets(line, sizeof(line)-1, fp)) return false;

    while (fgets(line, sizeof(line)-1, fp))
    {
        char* line_p = line;
        // 去掉前导空格
        while ((' ' == *line_p) || ('\t' == *line_p))
            ++line_p;
        
        char* colon = strchr(line_p, ':');
        if (NULL == colon) break;
        *colon = '\t';

        if ((interface_name != NULL)
         && (strncmp(line_p, interface_name, strlen(interface_name)) != 0))
            continue;
        
        net_info_t net_info;
        if (sscanf(line_p
                      ,"%s"
                       "%lu%lu%lu%lu%lu"
                       "%lu%lu%lu%lu%lu"
                       "%lu%lu%lu%lu%lu"
                       "%lu"
            /** 01 */ , net_info.interface_name
            /** 02 */ ,&net_info.receive_bytes
            /** 03 */ ,&net_info.receive_packets
            /** 04 */ ,&net_info.receive_errors
            /** 05 */ ,&net_info.receive_dropped
            /** 06 */ ,&net_info.receive_fifo_errors
            /** 07 */ ,&net_info.receive_frame
            /** 08 */ ,&net_info.receive_compressed
            /** 09 */ ,&net_info.receive_multicast
            /** 10 */ ,&net_info.transmit_bytes
            /** 11 */ ,&net_info.transmit_packets
            /** 12 */ ,&net_info.transmit_errors
            /** 13 */ ,&net_info.transmit_dropped
            /** 14 */ ,&net_info.transmit_fifo_errors
            /** 15 */ ,&net_info.transmit_collisions
            /** 16 */ ,&net_info.transmit_carrier
            /** 17 */ ,&net_info.transmit_compressed
                      ) != filed_number)
            break;

        net_info_array.push_back(net_info);
        return true;
    }

    return false;
}

bool CInfo::get_net_info(const char* interface_name, net_info_t& net_info)
{
    std::vector<net_info_t> net_info_array;
    if (do_get_net_info_array(interface_name, net_info_array))
    {
        memcpy(&net_info, &net_info_array[0], sizeof(net_info));
        return true;
    }

    return false;
}

bool CInfo::get_net_info_array(std::vector<net_info_t>& net_info_array)
{
    return do_get_net_info_array(NULL, net_info_array);
}

SYS_NAMESPACE_END
