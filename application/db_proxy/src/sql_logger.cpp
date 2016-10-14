// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "sql_logger.h"
#include "config_loader.h"
#include <algorithm>
#include <fcntl.h>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/log.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

INTEGER_ARG_DECLARE(int, sql_file_size);
namespace mooon { namespace db_proxy {

static __thread int stg_log_fd = -1;
static __thread int stg_old_log_fd = -1;

CSqlLogger::CSqlLogger(int database_index, const struct DbInfo* dbinfo)
    : _database_index(database_index)
{
    _log_file_timestamp = 0;
    _log_file_suffix = 0;
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
                MYLOG_INFO("[%s] try to rotate sql log: log_fd=%d, stg_log_fd=%d, stg_old_log_fd=%d\n", _dbinfo->str().c_str(), log_fd, stg_log_fd, stg_old_log_fd);
                rotate_log(true);
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

        int32_t length = static_cast<int32_t>(sql.size());
        struct iovec iov[2];
        iov[0].iov_base = &length;
        iov[0].iov_len = sizeof(length);
        iov[1].iov_base = const_cast<char*>(sql.data());
        iov[1].iov_len = sql.size();
        //ssize_t bytes_written = write(stg_log_fd, sql.data(), sql.size());
        ssize_t bytes_written = writev(stg_log_fd, iov, sizeof(iov)/sizeof(iov[0]));
        //if (bytes_written != static_cast<ssize_t>(sql.size()))
        if (bytes_written != static_cast<int>(iov[0].iov_len+iov[1].iov_len))
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
                int new_total_bytes_written = atomic_read(&_total_bytes_written);
                // 如果其它线程滚动了，则新读到的将和前面一步可能不同，只要参数sql_file_size足够大，基本可以保证下一句的判断成立

                if ((log_fd == stg_old_log_fd) && (new_total_bytes_written >= total_bytes_written))
                {
                    // 写结束标志
                    write_endtag();

                    MYLOG_INFO("[%s] try to rotate sql log: total_bytes_written=%d, new_total_bytes_written=%d, log_fd=%d, stg_log_fd=%d, stg_old_log_fd=%d\n", _dbinfo->str().c_str(), total_bytes_written, new_total_bytes_written, log_fd, stg_log_fd, stg_old_log_fd);
                    rotate_log(false);
                }
                else
                {
                    // 进入锁之前，其它线程已抢先做了滚动，以下特征一定标记着被其它线程滚动了：
                    // 1. 大小变小了
                    // 2. log_fd值发生了变化
                    MYLOG_INFO("[%s] sql log rotated by other: total_bytes_written=%d, new_total_bytes_written=%d, log_fd=%d, stg_log_fd=%d, stg_old_log_fd=%d\n", _dbinfo->str().c_str(), total_bytes_written, new_total_bytes_written, log_fd, stg_log_fd, stg_old_log_fd);
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

void CSqlLogger::write_endtag()
{
    int32_t length = 0;
    if (write(stg_log_fd, &length, sizeof(length)) != sizeof(length))
    {
        const std::string log_filepath = get_log_filepath();
        MYLOG_ERROR("[%s][%s] IO error: %s\n", _dbinfo->str().c_str(), log_filepath.c_str(), sys::Error::to_string().c_str());
    }
}

void CSqlLogger::rotate_log(bool boot)
{
    const std::string log_filepath = boot? get_last_log_filepath(): get_log_filepath();
    int new_log_fd = open(log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM); // O_EXCL

    if (-1 == new_log_fd)
    {
        MYLOG_ERROR("[%s] create log[%s] error: %s\n", _dbinfo->str().c_str(), log_filepath.c_str(), sys::Error::to_string().c_str());
    }
    else
    {
        // 关闭old_log_fd一定要放在open之后，以确保write_log中判断被其它线程抢先滚动能够成立，原因是open会重用最近一次被close的fd
        int old_log_fd = atomic_read(&_log_fd);
        if (old_log_fd != -1)
        {
            close(old_log_fd);
            MYLOG_DEBUG("close log_fd: (%d)%s\n", old_log_fd, _dbinfo->str().c_str());
            atomic_set(&_log_fd, -1);
        }
        if (stg_log_fd != -1)
        {
            close(stg_log_fd);
            MYLOG_DEBUG("[%s] close stg_log_fd: %s\n", _dbinfo->str().c_str(), log_filepath.c_str());
            stg_log_fd = -1;
        }

        int log_file_size = static_cast<int>(sys::CFileUtils::get_file_size(new_log_fd));
        atomic_set(&_total_bytes_written, log_file_size);
        atomic_set(&_log_fd, new_log_fd);
        stg_log_fd = dup(new_log_fd);
        stg_old_log_fd = new_log_fd;
        MYLOG_INFO("[%s] rotate and create new log file: (log_file_size=%d, new_log_fd=%d, old_log_fd=%d, stg_log_fd=%d, stg_old_log_fd=%d)%s\n", _dbinfo->str().c_str(), log_file_size, new_log_fd, old_log_fd, stg_log_fd, stg_old_log_fd, log_filepath.c_str());
    }
}

std::string CSqlLogger::get_log_filepath()
{
    std::string log_filepath;

    MOOON_ASSERT(!_dbinfo->alias.empty());
    if (_dbinfo->alias.empty())
    {
        MYLOG_ERROR("alias empty: %s\n", _dbinfo->str().c_str());
    }
    else
    {
        const std::string log_dirpath = get_log_dirpath(_dbinfo->alias);
        if (!sys::CDirUtils::exist(log_dirpath))
        {
            MYLOG_INFO("to create sqllog dir[%s]: %s\n", log_dirpath.c_str(), _dbinfo->str().c_str());
            sys::CDirUtils::create_directory_recursive(log_dirpath.c_str(), DIRECTORY_DEFAULT_PERM);
        }

        time_t now = time(NULL);
        if (now == _log_file_timestamp)
        {
            ++_log_file_suffix;
        }
        else
        {
            _log_file_suffix = 0;
            _log_file_timestamp = now;
        }
        log_filepath = utils::CStringUtils::format_string("%s/sql.%012" PRId64".%06d", log_dirpath.c_str(), static_cast<int64_t>(now), _log_file_suffix);
    }

    return log_filepath;
}

std::string CSqlLogger::get_last_log_filepath()
{
    std::vector<std::string>* subdir_names = NULL;
    std::vector<std::string>* link_names = NULL;
    std::vector<std::string> file_names;
    std::string log_filename;
    const std::string log_dirpath = get_log_dirpath(_dbinfo->alias);

    sys::CDirUtils::list(log_dirpath, subdir_names, &file_names, link_names);
    if (!file_names.empty())
    {
        std::sort(file_names.begin(), file_names.end());
        for (int i=static_cast<int>(file_names.size()-1); i>=0; --i)
        {
            if (is_sql_log_filename(file_names[i]))
            {
                log_filename = file_names[i];
                break;
            }
        }
    }

    if (log_filename.empty())
    {
        MYLOG_INFO("no history log file: %s\n", _dbinfo->str().c_str());
        return get_log_filepath();
    }
    else
    {
        std::string last_log_filepath = log_dirpath + std::string("/") + log_filename;

        // 检查是否为一个已完成文件
        if (!has_endtag(last_log_filepath))
        {
            MYLOG_INFO("[%s] history log file exists: %s\n", _dbinfo->str().c_str(), last_log_filepath.c_str());
        }
        else
        {
            last_log_filepath = get_log_filepath();
            MYLOG_INFO("new log file: %s\n", last_log_filepath.c_str());
        }

        return last_log_filepath;
    }
}

bool CSqlLogger::has_endtag(const std::string& log_filepath) const
{
    int fd = open(log_filepath.c_str(), O_RDONLY);
    if (-1 == fd)
    {
        MYLOG_ERROR("open [%s] error: %s\n", log_filepath.c_str(), sys::Error::to_string().c_str());
        return false;
    }
    else
    {
        sys::CloseHelper<int> close_helper(fd);

        while (true)
        {
            int32_t length = 0;
            int bytes_read = read(fd, &length, sizeof(length));
            if (0 == bytes_read)
            {
                break;
            }
            if (bytes_read != sizeof(length))
            {
                MYLOG_ERROR("read [%s] error: (%d)%s\n", log_filepath.c_str(), bytes_read, sys::Error::to_string().c_str());
                break;
            }
            else if (0 == length)
            {
                // found endtag
                MYLOG_INFO("[%s] has endtag\n", log_filepath.c_str());
                return true;
            }
            else
            {
                if (-1 == lseek(fd, length, SEEK_CUR))
                {
                    MYLOG_ERROR("lseek [%s] error: (%d)%s\n", log_filepath.c_str(), bytes_read, sys::Error::to_string().c_str());
                }
            }
        }

        MYLOG_INFO("[%s] without endtag\n", log_filepath.c_str());
        return false;
    }
}

} // namespace db_proxy
} // namespace mooon
