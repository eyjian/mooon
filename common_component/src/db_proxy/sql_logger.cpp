// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "sql_logger.h"
#include "config_loader.h"
#include <fcntl.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/log.h>
#include <mooon/sys/utils.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace mooon { namespace db_proxy {

// 线程级别的
static __thread int sg_thread_sql_log_fd = -1;

SINGLETON_IMPLEMENT(CSqlLogger)
uint32_t CSqlLogger::sql_log_filesize = 5242880; // 500MB

void CSqlLogger::write_log(int database_index, const std::string& sql)
{
    int fd = get_fd(database_index);
    if (-1 == fd)
    {
        MYLOG_ERROR("[SQL]%s\n", sql.c_str());
    }
    else
    {
        int bytes = write(fd, sql.c_str(), sql.size());

        if (bytes != static_cast<int>(sql.size()))
        {
            MYLOG_ERROR("write error: %s\n", strerror(errno));
        }

        if (need_roll(fd))
        {
            sys::LockHelper<sys::CLock> lock_helper(_lock);
            if (need_roll(fd))
            {
                roll(database_index);
            }
        }
    }
}

int CSqlLogger::get_fd(int database_index)
{
    if (-1 == sg_thread_sql_log_fd)
        sg_thread_sql_log_fd = open_sql_log(database_index);

    return sg_thread_sql_log_fd;
}

int CSqlLogger::open_sql_log(const std::string& current_filepath)
{
    int fd = -1;

    for (int i=0; i<2; ++i)
    {
        fd = open(current_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM);
        if (fd != -1)
        {
            MYLOG_INFO("open %s ok\n", current_filepath.c_str());
            break;
        }
        else
        {
            // if/else是为优化日志输出
            if ((0 == i) && sys::Error::is_not(ENOENT)) // No such file or directory
            {
                MYLOG_ERROR("open %s error: (%d)%s\n", current_filepath.c_str(), sys::Error::code(), sys::Error::to_string().c_str());
            }
            else if (1 == i)
            {
                MYLOG_ERROR("open %s error: (%d)%s\n", current_filepath.c_str(), sys::Error::code(), sys::Error::to_string().c_str());
            }
            else if (0 == i)
            {
                try
                {
                    // 目录不存在时尝试创建一下
                    sys::CDirUtils::create_directory_byfilepath(current_filepath.c_str(), DIRECTORY_DEFAULT_PERM);
                }
                catch (sys::CSyscallException& syscall_ex)
                {
                    MYLOG_ERROR("create directory by [%s] error: %s\n", current_filepath.c_str(), syscall_ex.str().c_str());
                }
            }

            // 试图打开时，其它线程可能正在滚动操作，所以稍后重试一次
            sys::CUtils::millisleep(100u);
        }
    }

    return fd;
}

int CSqlLogger::open_sql_log(int database_index)
{
    int fd = -1;
    struct DbInfo db_info;

    if (!CConfigLoader::get_singleton()->get_db_info(database_index, &db_info))
    {
        MYLOG_ERROR("get_db_info failed: db[%d] not exist\n", database_index);
    }
    else
    {
        std::string sql_log_filepath = get_current_filepath(db_info);
        fd = open_sql_log(sql_log_filepath);
    }

    return fd;
}

// homedir/sql_log/db_host_db_name/current/db_host_db_name.sql
std::string CSqlLogger::get_current_filepath(const struct DbInfo& db_info) const
{
    std::string homedir = sys::CUtils::get_program_path() + std::string("/..");
    std::string filepath = homedir + std::string("/sql_log/") +
                           db_info.host + std::string("_") + db_info.name + std::string("/current/") +
                           db_info.host + std::string("_") + db_info.name + std::string(".sql");

    return filepath;
}

// homedir/sql_log/db_host_db_name/current/db_host_db_name_YYYYMMDDHHmmss.sql
std::string CSqlLogger::get_rolled_filepath(const struct DbInfo& db_info, const std::string& datetime) const
{
    std::string homedir = sys::CUtils::get_program_path() + std::string("/..");
    std::string filepath = homedir + std::string("/sql_log/") +
                           db_info.host + std::string("_") + db_info.name + std::string("/current/") +
                           db_info.host + std::string("_") + db_info.name + std::string("_") + datetime + std::string(".sql");

    return filepath;
}

bool CSqlLogger::need_roll(int fd) const
{
    off_t file_size = sys::CFileUtils::get_file_size(fd);
    return file_size >= static_cast<off_t>(CSqlLogger::sql_log_filesize);
}

void CSqlLogger::roll(int database_index)
{
    struct DbInfo db_info;

    if (CConfigLoader::get_singleton()->get_db_info(database_index, &db_info))
    {
        std::string datetime = sys::CDatetimeUtils::get_current_datetime("%04d%02d%02d%02d%02d%02d");
        std::string current_filepath = get_current_filepath(db_info);
        std::string rotated_filepath = get_rolled_filepath(db_info, datetime);

        try
        {
            sys::CDirUtils::create_directory_byfilepath(rotated_filepath.c_str(), DIRECTORY_DEFAULT_PERM);
            sys::CFileUtils::rename(current_filepath.c_str(), rotated_filepath.c_str());

            int fd = open_sql_log(current_filepath);
            if (fd != -1)
                sg_thread_sql_log_fd = fd;
        }
        catch (sys::CSyscallException& syscall_ex)
        {
            MYLOG_ERROR("rename [%s] to [%s] failed: %s\n",
                current_filepath.c_str(), rotated_filepath.c_str(), sys::Error::to_string().c_str());
        }
    }
}

} // namespace db_proxy
} // namespace mooon
