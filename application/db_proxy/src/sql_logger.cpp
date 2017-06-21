// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "sql_logger.h"
#include "config_loader.h"
#include <algorithm>
#include <fcntl.h>
#include <mooon/observer/observer_manager.h>
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

INTEGER_ARG_DECLARE(int32_t, sql_file_size);
INTEGER_ARG_DECLARE(int32_t, lines);
namespace mooon { namespace db_proxy {

CSqlLogger::CSqlLogger(int database_index, const struct DbInfo* dbinfo)
    : _database_index(database_index), _log_fd(-1)
{
    _log_file_timestamp = 0;
    _log_file_suffix = 0;
    _num_lines = 0;
    _total_lines = 0;
    _last_total_lines = 0;
    _dbinfo = new struct DbInfo(dbinfo);

    mooon::observer::IObserverManager* observer_mananger = mooon::observer::get();
    if (observer_mananger != NULL)
        observer_mananger->register_observee(this);
}

CSqlLogger::~CSqlLogger()
{
    mooon::observer::IObserverManager* observer_mananger = mooon::observer::get();
    if (observer_mananger != NULL)
        observer_mananger->deregister_objservee(this);

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
        int32_t length = static_cast<int32_t>(sql.size());
        sys::LockHelper<sys::CLock> lock_helper(_lock);

        if ((-1 == _log_fd) || need_rotate())
        {
            rotate_log();
        }
        if (-1 == _log_fd)
        {
            return false;
        }

        struct iovec iov[2];
        iov[0].iov_base = &length;
        iov[0].iov_len = sizeof(length);
        iov[1].iov_base = const_cast<char*>(sql.data());
        iov[1].iov_len = sql.size();
        ssize_t bytes_written = writev(_log_fd, iov, sizeof(iov)/sizeof(iov[0]));
        if (bytes_written != static_cast<int>(iov[0].iov_len+iov[1].iov_len))
        {
            const int errcode = errno;
            THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("writev %s error: %s", _log_filepath.c_str(), sys::Error::to_string(errcode).c_str()), errcode, "writev");
        }

        // 计数
        ++_total_lines;

        const int32_t lines = argument::lines->value();
        if ((lines > 0) && (++_num_lines >= lines))
        {
            _num_lines = 0;
            if (-1 == fdatasync(_log_fd))
            {
                const int errcode = errno;
                THROW_SYSCALL_EXCEPTION(utils::CStringUtils::format_string("fdatasync %s error: %s", _log_filepath.c_str(), sys::Error::to_string(errcode).c_str()), errno, "fdatasync");
            }
        }

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        MYLOG_ERROR("[%s] write [%s] to %s failed: %s\n", _dbinfo->str().c_str(), sql.c_str(), _log_filepath.c_str(), ex.str().c_str());
        return false;
    }
}

bool CSqlLogger::need_rotate() const
{
    const int32_t file_size = static_cast<int32_t>(sys::CFileUtils::get_file_size(_log_fd));
    return file_size >= argument::sql_file_size->value();
}

void CSqlLogger::rotate_log()
{
    if (_log_fd != -1)
    {
        if (-1 == fsync(_log_fd))
        {
            const int errcode = errno;
            MYLOG_ERROR("fsync %s error: (%d)%s\n", _log_filepath.c_str(), errcode, sys::Error::to_string(errcode).c_str());
        }

        close(_log_fd);
        _log_fd = -1;
    }
    if (_log_filepath.empty())
    {
        _log_filepath = get_last_log_filepath();
        MYLOG_INFO("%s\n", _log_filepath.c_str());
    }
    else
    {
        _log_filepath = get_log_filepath();
        MYLOG_INFO("%s\n", _log_filepath.c_str());
    }

    if (!_log_filepath.empty())
    {
        _log_fd = open(_log_filepath.c_str(), O_WRONLY|O_CREAT|O_APPEND, FILE_DEFAULT_PERM); // O_EXCL
        if (-1 == _log_fd)
        {
            MYLOG_ERROR("[%s] create %s error: %s\n", _dbinfo->str().c_str(), _log_filepath.c_str(), sys::Error::to_string().c_str());
        }
        else
        {
            int log_file_size = static_cast<int>(sys::CFileUtils::get_file_size(_log_fd));
            MYLOG_INFO("[%s] create %s ok: %d\n", _dbinfo->str().c_str(), _log_filepath.c_str(), log_file_size);
        }
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
        return last_log_filepath;
    }
}

void CSqlLogger::on_report(mooon::observer::IDataReporter* data_reporter, const std::string& current_datetime)
{
    if ((_total_lines > 0) && (_total_lines > _last_total_lines))
    {
        _last_total_lines = _total_lines;
        data_reporter->report("[%s][D]%d,%" PRIu64"\n", current_datetime.c_str(), _database_index, _total_lines);
    }
}

} // namespace db_proxy
} // namespace mooon
