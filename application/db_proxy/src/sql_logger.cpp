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
static __thread std::string* stg_log_filepath = NULL;

CSqlLogger::CSqlLogger(int database_index, const struct DbInfo* dbinfo)
    : _database_index(database_index)
{
    _log_file_timestamp = 0;
    _log_file_suffix = 0;
    _dbinfo = new struct DbInfo(dbinfo);
}

CSqlLogger::~CSqlLogger()
{
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
        if (-1 == stg_log_fd)
        {
            sys::LockHelper<sys::CLock> lock_helper(_lock);
            rotate_log();
            if (-1 == stg_log_fd)
            {
                return false;
            }
        }

        int32_t length = static_cast<int32_t>(sql.size());
        struct iovec iov[2];
        iov[0].iov_base = &length;
        iov[0].iov_len = sizeof(length);
        iov[1].iov_base = const_cast<char*>(sql.data());
        iov[1].iov_len = sql.size();
        ssize_t bytes_written = writev(stg_log_fd, iov, sizeof(iov)/sizeof(iov[0]));
        if (bytes_written != static_cast<int>(iov[0].iov_len+iov[1].iov_len))
        {
            int errcode = errno;
            sys::LockHelper<sys::CLock> lock_helper(_lock);
            MYLOG_ERROR("write %s error(%d): %s\n", stg_log_filepath->c_str(), stg_log_fd, sys::Error::to_string(errcode).c_str());
            return false;
        }
        else
        {
            if (need_rotate_by_filesize()) // 达到或超过指定的文件大小
            {
                sys::LockHelper<sys::CLock> lock_helper(_lock);

                if (!need_rotate_by_filename()) // 文件名发生变化表示其它线程抢先滚动了
                {
                    open_log();
                }
                else // 文件名未变表示还没有滚动
                {
                    write_endtag(); // 写结束标志
                    rotate_log();
                }
            }

            return true;
        }
    }
    catch (sys::CSyscallException& ex)
    {
        if (NULL == stg_log_filepath)
        {
            MYLOG_ERROR("[%s] write failed: %s\n", _dbinfo->str().c_str(), ex.str().c_str());
        }
        else
        {
            MYLOG_ERROR("[%s] write %s failed: %s\n", _dbinfo->str().c_str(), stg_log_filepath->c_str(), ex.str().c_str());
        }

        return false;
    }
}

bool CSqlLogger::need_rotate_by_filesize() const
{
    int file_size = static_cast<int>(sys::CFileUtils::get_file_size(stg_log_fd));
    return file_size >= argument::sql_file_size->value();
}

bool CSqlLogger::need_rotate_by_filename() const
{
    return _log_filepath == *stg_log_filepath;
}

void CSqlLogger::write_endtag()
{
    int32_t length = 0; // 当遇到length为0时表示结束
    if (write(stg_log_fd, &length, sizeof(length)) != static_cast<ssize_t>(sizeof(length)))
    {
        MYLOG_ERROR("[%s][%s] IO error: %s\n", _dbinfo->str().c_str(), _log_filepath.c_str(), sys::Error::to_string().c_str());
    }
}

void CSqlLogger::rotate_log()
{
    if (stg_log_fd != -1)
        close(stg_log_fd);

    if (_log_filepath.empty())
        _log_filepath = get_last_log_filepath();
    else
        _log_filepath = get_log_filepath();

    stg_log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM); // O_EXCL
    if (-1 == stg_log_fd)
    {
        MYLOG_ERROR("[%s] create %s error: %s\n", _dbinfo->str().c_str(), _log_filepath.c_str(), sys::Error::to_string().c_str());
    }
    else
    {
        int log_file_size = static_cast<int>(sys::CFileUtils::get_file_size(stg_log_fd));
        MYLOG_INFO("[%s] create %s ok: %d\n", _dbinfo->str().c_str(), _log_filepath.c_str(), log_file_size);
    }

    if (NULL == stg_log_filepath)
    {
        stg_log_filepath = new std::string;
        MYLOG_DEBUG("stg_log_filepath instantiated: %p\n", stg_log_filepath);
    }
    *stg_log_filepath = _log_filepath;
}

void CSqlLogger::open_log()
{
    if (stg_log_fd != -1)
        close(stg_log_fd);

    stg_log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM); // O_EXCL
    if (-1 == stg_log_fd)
    {
        MYLOG_ERROR("[%s] open %s error: %s\n", _dbinfo->str().c_str(), _log_filepath.c_str(), sys::Error::to_string().c_str());
    }
    else
    {
        MYLOG_INFO("[%s] open %s ok\n", _dbinfo->str().c_str(), _log_filepath.c_str());
    }

    *stg_log_filepath = _log_filepath;
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
        log_filepath = utils::CStringUtils::format_string("%s/sql.%013" PRId64".%06d", log_dirpath.c_str(), static_cast<int64_t>(now), _log_file_suffix);
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
