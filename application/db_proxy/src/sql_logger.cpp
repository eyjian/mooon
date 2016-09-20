// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "sql_logger.h"
#include "config_loader.h"
#include <fcntl.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/log.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <sys/stat.h>
#include <sys/types.h>

INTEGER_ARG_DECLARE(int, sql_file_size);
namespace mooon { namespace db_proxy {

static __thread int stg_log_fd = -1;
static __thread int stg_old_log_fd = -1;

CSqlLogger::CSqlLogger(int database_index, const struct DbInfo* dbinfo)
    : _database_index(database_index)
{
    atomic_set(&_log_fd, -1);
    _dbinfo = new struct DbInfo(dbinfo);
}

CSqlLogger::~CSqlLogger()
{
    int log_fd = atomic_read(&_log_fd);
    if (log_fd != -1)
        close(log_fd);
    atomic_set(&_log_fd, -2);
    delete _dbinfo;
}

std::string CSqlLogger::str() const
{
    return _dbinfo->str();
}

bool CSqlLogger::write_log(const std::string& sql)
{
    try
    {
        int log_fd = atomic_read(&_log_fd);
        MYLOG_DEBUG("[%s]: log_fd=%d, stg_log_fd=%d, stg_old_log_fd=%d\n", _dbinfo->str().c_str(), log_fd, stg_log_fd, stg_old_log_fd);
        if ((-1 == stg_log_fd) || (stg_old_log_fd != log_fd))
        {
            sys::LockHelper<sys::CLock> lock_helper(_lock);
            log_fd = atomic_read(&_log_fd); // 进入锁之前，可能其它线程已抢先做了滚动
            if (-1 == log_fd)
            {
                rotate_log();
            }
            else
            {
                if (stg_log_fd != -1)
                    close(stg_log_fd);
                stg_old_log_fd = log_fd;
                stg_log_fd = dup(log_fd);
                if (stg_log_fd != -1)
                {
                    MYLOG_DEBUG("dup ok: (%d)%s\n", stg_log_fd, _dbinfo->str().c_str());
                }
                else
                {
                    const std::string log_filepath = get_log_filepath();
                    MYLOG_ERROR("[%s] dup %s error: %s\n", _dbinfo->str().c_str(), log_filepath.c_str(), sys::Error::to_string().c_str());
                }
            }
        }
        if (-1 == stg_log_fd)
        {
            return false;
        }

        ssize_t bytes_written = write(stg_log_fd, sql.data(), sql.size());
        if (bytes_written != static_cast<ssize_t>(sql.size()))
        {
            int errcode = errno;
            sys::LockHelper<sys::CLock> lock_helper(_lock);
            const std::string log_filepath = get_log_filepath();
            MYLOG_ERROR("[%s][%s] write sql[%s] error: (bytes_written=%zd,stg_log_fd=%d)%s\n", _dbinfo->str().c_str(), log_filepath.c_str(), sql.c_str(), bytes_written, stg_log_fd, sys::Error::to_string(errcode).c_str());
            return false;
        }
        else
        {
            int total_bytes_written = atomic_add_return(bytes_written, &_total_bytes_written);
            if (total_bytes_written > mooon::argument::sql_file_size->value())
            {
                sys::LockHelper<sys::CLock> lock_helper(_lock);
                int log_fd = atomic_read(&_log_fd);
                MYLOG_DEBUG("[%s]: log_fd=%d, stg_old_log_fd=%d, stg_log_fd=%d\n", _dbinfo->str().c_str(), log_fd, stg_old_log_fd, stg_log_fd);
                if (log_fd == stg_old_log_fd)
                {
                    rotate_log();
                }
                else
                {
                    // 进入锁之前，其它线程已抢先做了滚动
                    MYLOG_INFO("sql log rotated by other: %s\n", _dbinfo->str().c_str());
                    close(stg_log_fd);
                    stg_log_fd = dup(log_fd);
                    stg_old_log_fd = log_fd;
                }
            }

            return true;
        }
    }
    catch (sys::CSyscallException& ex)
    {
        const std::string log_filepath = get_log_filepath();
        MYLOG_ERROR("[%s] write %s failed: %s\n", _dbinfo->str().c_str(), log_filepath.c_str(), ex.str().c_str());
        return false;
    }
}

void CSqlLogger::rotate_log()
{
    const std::string log_filepath = get_log_filepath();
    int log_fd = atomic_read(&_log_fd);

    if (log_fd != -1)
    {
        close(log_fd);
        MYLOG_DEBUG("close log_fd: (%d)%s\n", log_fd, _dbinfo->str().c_str());
        atomic_set(&_log_fd, -1);
    }
    if (stg_log_fd != -1)
    {
        close(stg_log_fd);
        MYLOG_DEBUG("[%s] close stg_log_fd: %s\n", _dbinfo->str().c_str(), log_filepath.c_str());
        stg_log_fd = -1;
    }

    log_fd = open(log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND|O_EXCL, FILE_DEFAULT_PERM);
    if (-1 == log_fd)
    {
        MYLOG_ERROR("[%s] create log[%s] error: %s\n", _dbinfo->str().c_str(), log_filepath.c_str(), sys::Error::to_string().c_str());
    }
    else
    {
        off_t log_file_size = sys::CFileUtils::get_file_size(log_fd);
        atomic_set(&_total_bytes_written, log_file_size);
        atomic_set(&_log_fd, log_fd);
        stg_log_fd = dup(log_fd);
        stg_old_log_fd = log_fd;
        MYLOG_INFO("[%s] rotate and create new log file: (log_fd=%d, stg_log_fd=%d)%s\n", _dbinfo->str().c_str(), log_fd, stg_log_fd, log_filepath.c_str());
    }
}

std::string CSqlLogger::get_log_filepath() const
{
    std::string log_filepath;

    MOOON_ASSERT(!_dbinfo->alias.empty());
    if (_dbinfo->alias.empty())
    {
        MYLOG_ERROR("alias empty: %s\n", _dbinfo->str().c_str());
    }
    else
    {
        const std::string program_path = sys::CUtils::get_program_path();
        std::string log_dirpath = program_path + std::string("/../sql_log");
        if (!sys::CDirUtils::exist(log_dirpath))
        {
            MYLOG_WARN("[%s][%s] not exist to use %s to store sql log\n", _dbinfo->str().c_str(), log_dirpath.c_str(), program_path.c_str());
            log_dirpath = program_path;
        }

        log_dirpath = log_dirpath + std::string("/") + _dbinfo->alias;
        if (!sys::CDirUtils::exist(log_dirpath))
        {
            sys::CDirUtils::create_directory(log_dirpath.c_str(), DIRECTORY_DEFAULT_PERM);
        }

        time_t now = time(NULL);
        log_filepath = utils::CStringUtils::format_string("%s/sql.%" PRId64, log_dirpath.c_str(), static_cast<int64_t>(now));
    }

    return log_filepath;
}

} // namespace db_proxy
} // namespace mooon
