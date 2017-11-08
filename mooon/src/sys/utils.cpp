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
 * Author: jian yi, eyjian@qq.com or eyjian@gmail.com
 */
#include "sys/utils.h"
#include "sys/atomic.h"
#include "sys/close_helper.h"
#include "utils/string_utils.h"
#include <dirent.h>
#include <execinfo.h> // backtrace和backtrace_symbols函数
#include <features.h> // feature_test_macros
#include <ftw.h> // ftw
#include <libgen.h> // dirname&basename
#include <pwd.h> // getpwuid
#include <sys/time.h>
#include <sys/prctl.h> // prctl
#include <sys/resource.h>
#include <sys/types.h>
#include <time.h>

#ifndef PR_SET_NAME
#define PR_SET_NAME 15
#endif
 
#ifndef PR_GET_NAME
#define PR_GET_NAME 16
#endif

////////////////////////////////////////////////////////////////////////////////
SYS_NAMESPACE_BEGIN

// 和set_program_title相关
static char *g_arg_start = NULL;
static char *g_arg_end   = NULL;
static char *g_env_start = NULL;

void CUtils::millisleep(uint32_t milliseconds)
{
    struct timespec ts = { milliseconds / 1000, (milliseconds % 1000) * 1000000 };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

void CUtils::microsleep(uint32_t microseconds)
{
    struct timespec ts = { microseconds / 1000000, (microseconds % 1000000) * 1000 };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

std::string CUtils::get_error_message(int errcode)
{
    return Error::to_string(errcode);
}

std::string CUtils::get_last_error_message()
{
    return Error::to_string();
}

int CUtils::get_last_error_code()
{
    return Error::code();
}

uint32_t CUtils::get_current_process_id()
{
    return static_cast<uint32_t>(getpid());
}

uint32_t CUtils::get_current_userid()
{
    return static_cast<uint32_t>(getuid());
}

std::string CUtils::get_current_username()
{
    char *buf;
    struct passwd pwd;
    struct passwd* result;
    std::string username;

    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (-1 == bufsize)
    {
        bufsize = 16384;        /* Should be more than enough */
    }

    buf = new char[bufsize];
    const uint32_t uid = get_current_userid();
    getpwuid_r(uid, &pwd, buf, bufsize-1, &result);
    if (result != NULL)
    {
        username = pwd.pw_name;
    }

    delete []buf;
    return username;
}

std::string CUtils::get_program_fullpath()
{
    const size_t bufsize = PATH_MAX;
    char* buf = new char[bufsize];
    const int retval = readlink("/proc/self/exe", buf, bufsize-1);
    if (retval > 0)
    {
        buf[retval] = '\0';
    }
    else
    {
        *buf = '\0';
    }

    const std::string fullpath(buf);
    delete []buf;
    return fullpath;
}

std::string CUtils::get_program_path()
{
    return get_program_dirpath();
}

std::string CUtils::get_program_dirpath()
{
    const size_t bufsize = PATH_MAX;
    char* buf = new char[bufsize];
    const int n = readlink("/proc/self/exe", buf, bufsize-1);

    if (n > 0)
    {
        buf[n] = '\0';

#if 0 // 保留这段废代码，以牢记deleted的存在，但由于这里只取路径部分，所以不关心它的存在
        if (!strcmp(buf+retval-10," (deleted)"))
            buf[retval-10] = '\0';
#else

        // 去掉文件名部分
        char* end = strrchr(buf, '/');
        if (NULL == end)
            buf[0] = 0;
        else
            *end = '\0';
#endif
    }
    else
    {
        buf[0] = '\0';
    }

    const std::string dirpath = buf;
    delete []buf;
    return dirpath;
}

int CUtils::get_program_parameters(std::vector<std::string>* parameters, uint32_t pid)
{
    char* raw_full_cmdline_buf = new char[PATH_MAX];

    std::string cmdline_filepath;
    if (0 == pid)
        cmdline_filepath = "/proc/self/cmdline";
    else
        cmdline_filepath = utils::CStringUtils::format_string("/proc/%u/cmdline", pid);

    const int fd = open(cmdline_filepath.c_str(), O_RDONLY);
    if (fd != -1)
    {
        const int n = read(fd, raw_full_cmdline_buf, PATH_MAX-1);
        if (n != -1)
        {
            raw_full_cmdline_buf[n] = '\0';

            std::string parameter;
            for (int i=0; i<n; ++i)
            {
                if ('\0' == raw_full_cmdline_buf[i])
                {
                    parameters->push_back(parameter);
                    parameter.clear();
                }
                else
                {
                    parameter.push_back(raw_full_cmdline_buf[i]);
                }
            }
        }

        close(fd);
    }

    delete []raw_full_cmdline_buf;
    return static_cast<int>(parameters->size());
}

std::string CUtils::get_program_full_cmdline(char separator, uint32_t pid)
{
    std::string full_cmdline;
    std::vector<std::string> parameters;

    const int n = get_program_parameters(&parameters, pid);
    for (int i=0; i<n; ++i)
    {
        if (full_cmdline.empty())
        {
            full_cmdline = parameters[i];
        }
        else
        {
            full_cmdline.push_back(separator);
            full_cmdline.append(parameters[i]);
        }
    }

    return full_cmdline;
}

std::string CUtils::get_filename(int fd)
{
	char* path_buf = new char[PATH_MAX];
	char* filename_buf = new char[FILENAME_MAX];
	*filename_buf = '\0';
	
	snprintf(path_buf, PATH_MAX-1, "/proc/%d/fd/%d", getpid(), fd);
	if (-1 == readlink(path_buf, filename_buf, FILENAME_MAX-1)) filename_buf[0] = '\0';
    
	const std::string filename = filename_buf;
	delete []filename_buf;
	delete []path_buf;
	return filename;
}

// 库函数：char *realpath(const char *path, char *resolved_path);
//         char *canonicalize_file_name(const char *path);
std::string CUtils::get_full_directory(const char* directory)
{
    std::string full_directory;
    DIR* dir = opendir(directory);
    if (dir != NULL)
    {
        int fd = dirfd(dir);
        if (fd != -1)
            full_directory = get_filename(fd);

        closedir(dir);
    }
 
    return full_directory;
}

// 相关函数：
// get_nprocs()，声明在sys/sysinfo.h
// sysconf(_SC_NPROCESSORS_CONF)
// sysconf(_SC_NPROCESSORS_ONLN)
uint16_t CUtils::get_cpu_number()
{
	FILE* fp = fopen("/proc/cpuinfo", "r");
	if (NULL == fp) return 1;
	
	char line[LINE_MAX];
	uint16_t cpu_number = 0;
    sys::CloseHelper<FILE*> ch(fp);

	while (fgets(line, sizeof(line)-1, fp))
	{
		char* name = line;
		char* value = strchr(line, ':');
		
		if (NULL == value)
			continue;

		*value++ = 0;		
		if (0 == strncmp("processor", name, sizeof("processor")-1))
		{
			 if (!utils::CStringUtils::string2uint16(value, cpu_number))
             {
                 return 0;
             }
		}
	}

	return (cpu_number+1);
}

bool CUtils::get_backtrace(std::string& call_stack)
{
    const int frame_number_max = 20;       // 最大帧层数
    void* address_array[frame_number_max]; // 帧地址数组

    // real_frame_number的值不会超过frame_number_max，如果它等于frame_number_max，则表示顶层帧被截断了
    int real_frame_number = backtrace(address_array, frame_number_max);

    char** symbols_strings = backtrace_symbols(address_array, real_frame_number);
    if (NULL == symbols_strings)
    {
        return false;
    }
    else if (real_frame_number < 2) 
    {
        free(symbols_strings);
        return false;
    }

    call_stack = symbols_strings[1]; // symbols_strings[0]为get_backtrace自己，不显示
    for (int i=2; i<real_frame_number; ++i)
    {
        call_stack += std::string("\n") + symbols_strings[i];
    }

    free(symbols_strings);
    return true;
}

static off_t dirsize; // 目录大小
int _du_fn(const char *fpath, const struct stat *sb, int typeflag)
{   
    if (FTW_F == typeflag)
        dirsize += sb->st_size;

    return 0;
}

off_t CUtils::du(const char* dirpath)
{
    dirsize = 0;
    if (ftw(dirpath, _du_fn, 0) != 0) return -1;

    return dirsize;
}

int CUtils::get_page_size()
{
    // sysconf(_SC_PAGE_SIZE);
    // sysconf(_SC_PAGESIZE);
    return getpagesize();
}

int CUtils::get_fd_max()
{
    // sysconf(_SC_OPEN_MAX);
    return getdtablesize();
}

bool CUtils::is_file(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    return S_ISREG(buf.st_mode);
}

bool CUtils::is_file(const char* path)
{
    struct stat buf;
    if (-1 == stat(path, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "stat");

    return S_ISREG(buf.st_mode);
}

bool CUtils::is_link(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    return S_ISLNK(buf.st_mode);
}

bool CUtils::is_link(const char* path)
{
    struct stat buf;
    if (-1 == stat(path, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "stat");

    return S_ISLNK(buf.st_mode);
}

bool CUtils::is_directory(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "fstat");

    return S_ISDIR(buf.st_mode);
}

bool CUtils::is_directory(const char* path)
{
    struct stat buf;
    if (-1 == stat(path, &buf))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "stat");

    return S_ISDIR(buf.st_mode);
}

void CUtils::enable_core_dump(bool enabled, int core_file_size)
{    
    if (enabled)
    {
        struct rlimit rlim;
        rlim.rlim_cur = (core_file_size < 0)? RLIM_INFINITY: core_file_size;
        rlim.rlim_max = rlim.rlim_cur;

        if (-1 == setrlimit(RLIMIT_CORE, &rlim))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "setrlimit");
    }       
    
    if (-1 == prctl(PR_SET_DUMPABLE, enabled? 1: 0))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "prctl");
}

std::string CUtils::get_program_long_name()
{
    //#define _GNU_SOURCE
    //#include <errno.h>
    return program_invocation_name;
}

// 如果调用了set_process_title()，
// 则通过program_invocation_short_name可能取不到预期的值，甚至返回的是空
std::string CUtils::get_program_short_name()
{
    //#define _GNU_SOURCE
    //#include <errno.h>
    return program_invocation_short_name;
}

std::string CUtils::get_filename(const std::string& filepath)
{
    // basename的参数即是输入，也是输出参数，所以需要tmp_filepath
    std::string tmp_filepath(filepath);
    return basename(const_cast<char*>(tmp_filepath.c_str())); // #include <libgen.h>
}

std::string CUtils::get_dirpath(const std::string& filepath)
{
    // basename的参数即是输入，也是输出参数，所以需要tmp_filepath
    std::string tmp_filepath(filepath);
    return dirname(const_cast<char*>(tmp_filepath.c_str())); // #include <libgen.h>
}

void CUtils::set_process_name(const std::string& new_name)
{
    if (!new_name.empty())
    {
        if (-1 == prctl(PR_SET_NAME, new_name.c_str()))
            THROW_SYSCALL_EXCEPTION(NULL, errno, "prctl");
    }
}

void CUtils::set_process_name(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);

    char name[NAME_MAX];
    vsnprintf(name, sizeof(name), format, args);
    set_process_name(std::string(name));
}

// 参考：http://www.stamhe.com/?p=416
void CUtils::init_process_title(int argc, char *argv[])
{
    g_arg_start = argv[0];
    g_arg_end = argv[argc-1] + strlen(argv[argc-1]) + 1;
    g_env_start = environ[0];
}

void CUtils::set_process_title(const std::string &new_title)
{
    MOOON_ASSERT(g_arg_start != NULL);
    MOOON_ASSERT(g_arg_end != NULL);
    MOOON_ASSERT(g_env_start != NULL);

    if ((g_arg_start != NULL) && (g_arg_end != NULL) && (g_env_start != NULL) && !new_title.empty())
    {
        size_t new_title_len = new_title.length();

        // 新的title比老的长
        if ((static_cast<size_t>(g_arg_end-g_arg_start) < new_title_len)
         && (g_env_start == g_arg_end))
        {
            char *env_end = g_env_start;
            for (int i=0; environ[i]; ++i)
            {
                if (env_end != environ[i])
                {
                    break;
                }

                env_end = environ[i] + strlen(environ[i]) + 1;
                environ[i] = strdup(environ[i]);
            }

            g_arg_end = env_end;
            g_env_start = NULL;
        }

        size_t len = g_arg_end - g_arg_start;
        if (len == new_title_len)
        {
             strcpy(g_arg_start, new_title.c_str());
        }
        else if(new_title_len < len)
        {
            strcpy(g_arg_start, new_title.c_str());
            memset(g_arg_start+new_title_len, 0, len-new_title_len);

#if 0 // 从实际的测试来看，如果开启以下两句，会出现ps u输出的COMMAND一列为空
            // 当新的title比原title短时，
            // 填充argv[0]字段时，改为填充argv[0]区的后段，前段填充0
            memset(g_arg_start, 0, len);
            strcpy(g_arg_start+(len - new_title_len), new_title.c_str());
#endif
        }
        else
        {
            *(char *)mempcpy(g_arg_start, new_title.c_str(), len-1) = '\0';
        }

        if (g_env_start != NULL)
        {
            char *p = strchr(g_arg_start, ' ');
            if (p != NULL)
            {
                *p = '\0';
            }
        }
    }
}

void CUtils::set_process_title(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    utils::VaListHelper vlh(args);

    char title[PATH_MAX];
    vsnprintf(title, sizeof(title), format, args);
    set_process_title(std::string(title));
}

void CUtils::common_pipe_read(int fd, char** buffer, int32_t* buffer_size)
{
	int ret = 0;
	int32_t size = 0;

	// 第一个while循环读取大小
	while (true)
	{
		ret = read(fd, &size, sizeof(size));
		if ((-1 == ret) && (EINTR == errno))
			continue;
		if (-1 == ret)
		    THROW_SYSCALL_EXCEPTION(NULL, errno, "read");

		break;
	}

	*buffer_size = size;
	*buffer = new char[size];
	char* bufferp =  *buffer;

	// 第二个while循环根据大小读取内容
	while (size > 0)
	{
		ret = read(fd, bufferp, size);
		if ((0 == ret) || (ret == size))
			break;
		if ((-1 == ret) && (EINTR == errno))
			continue;
		if (-1 == ret)
		{
			delete *buffer;
			THROW_SYSCALL_EXCEPTION(NULL, errno, "read");
		}

		bufferp += ret;
		size -= ret;
	}
}

void CUtils::common_pipe_write(int fd, const char* buffer, int32_t buffer_size)
{
	int ret = 0;
	int32_t size = buffer_size;

	// 第一个while循环写入大小
	while (true)
	{
		ret = write(fd, &size, sizeof(size));
		if ((-1 == ret) && (EINTR == errno))
			continue;
		if (-1 == ret)
		    THROW_SYSCALL_EXCEPTION(NULL, errno, "write");

		break;
	}

	const char* bufferp = buffer;

	// 第二个while循环根据大小写入内容
	while (size > 0)
	{
		ret = write(fd, bufferp, size);
		if ((-1 == ret) && (EINTR == errno))
			continue;
		if (-1 == ret)
		    THROW_SYSCALL_EXCEPTION(NULL, errno, "write");

		size -= ret;
		bufferp += ret;
	}
}

std::string CUtils::get_random_string()
{
    // 随机数因子
    static atomic_t s_random_factor = ATOMIC_INIT(0);
    const uint32_t random_factor = static_cast<uint32_t>(atomic_read(&s_random_factor));

    // 取得当前时间
    struct timeval tv;
    struct timezone *tz = NULL;
    gettimeofday(&tv, tz);

    // 随机数1
    srandom(tv.tv_usec + random_factor + 0);
    uint64_t m1 = random() % std::numeric_limits<uint64_t>::max();

    // 随机数2
    srandom(tv.tv_usec + random_factor + 1);
    uint64_t m2 = random() % std::numeric_limits<uint64_t>::max();

    // 随机数3
    srandom(tv.tv_usec + random_factor + 2);
    uint64_t m3 = random() % std::numeric_limits<uint64_t>::max();

    atomic_add(3, &s_random_factor);
    return utils::CStringUtils::format_string("%" PRIu64"%" PRIu64"%u%" PRIu64"%" PRIu64"%" PRIu64, static_cast<uint64_t>(tv.tv_sec), static_cast<uint64_t>(tv.tv_usec), random_factor, m1, m2, m3);
}

SYS_NAMESPACE_END
