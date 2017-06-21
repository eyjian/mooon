// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "db_process.h"
#include <algorithm>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/file_utils.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <string.h>
#include <sys/uio.h>

INTEGER_ARG_DECLARE(uint16_t, report_frequency_seconds);
INTEGER_ARG_DECLARE(int32_t, sql_file_size);
INTEGER_ARG_DECLARE(uint16_t, port);
INTEGER_ARG_DECLARE(uint8_t, batch);
INTEGER_ARG_DECLARE(uint16_t, efficiency);
INTEGER_ARG_DECLARE(uint16_t, history_days);
INTEGER_ARG_DECLARE(uint8_t, history_hour);
INTEGER_ARG_DECLARE(uint8_t, auto_exit);
namespace mooon { namespace db_proxy {

CDbProcess::CDbProcess(const struct DbInfo& dbinfo)
    : _report_frequency_seconds(mooon::argument::report_frequency_seconds->value()),
      _progess_fd(-1), _dbinfo(dbinfo), _stop_signal_thread(false), _signal_thread(NULL),
      _consecutive_failures(0), _db_connected(false), _old_history_files_deleted_today(false)
{
    reset();

    const std::string program_path = sys::CUtils::get_program_path();
    _log_dirpath = get_log_dirpath(_dbinfo.alias);

    _begin_time = time(NULL);
    _interval_count = 0;
    _batch = 0;
}

CDbProcess::~CDbProcess()
{
    if (_report_frequency_seconds > 0)
    {
        mooon::observer::IObserverManager* observer_mananger = mooon::observer::get();
        if (observer_mananger != NULL)
            observer_mananger->deregister_objservee(this);
        mooon::observer::destroy();
    }

    if (_progess_fd != -1)
        close(_progess_fd);
    if (_signal_thread != NULL)
    {
        _signal_thread->join();
        delete _signal_thread;
    }
}

void CDbProcess::run()
{
    const uint16_t port = argument::port->value();
    const std::string port_str = utils::CStringUtils::int_tostring(port);
    const std::string log_dirpath = sys::get_log_dirpath(true);
    const std::string db_process_title = std::string("mdbp_") + _dbinfo.alias + std::string("_") + port_str; // mdbp: mooon db_proxy process
    sys::CUtils::set_process_title(db_process_title);

    delete sys::g_logger; // 不共享父进程的日志文件
    sys::g_logger = sys::create_safe_logger(log_dirpath, db_process_title, SIZE_8K);
    MYLOG_INFO("db_process(%u) started: %s, report_frequency_seconds: %d\n", getpid(), db_process_title.c_str(), _report_frequency_seconds);

    if (_report_frequency_seconds > 0)
    {
        observer::observer_logger = sys::g_logger;
        observer::reset(); // 得先释放父进程的

        std::string data_dirpath = mooon::observer::get_data_dirpath();
        if (data_dirpath.empty())
        {
            MYLOG_WARN("datadir not exists\n");
        }
        else
        {
            _data_logger.reset(new mooon::sys::CSafeLogger(data_dirpath.c_str(), utils::CStringUtils::format_string("%s.data", _dbinfo.alias.c_str()).c_str()));
            _data_logger->enable_raw_log(true);
            _data_logger->set_backup_number(2);
            _data_reporter.reset(new mooon::observer::CDefaultDataReporter(_data_logger.get()));

            observer::IObserverManager* observer_mananger = mooon::observer::create(_data_reporter.get(), _report_frequency_seconds);
            if (NULL == observer_mananger)
            {
                MYLOG_WARN("create observer mananger failed\n");
            }
            else
            {
                MYLOG_INFO("create observer mananger ok\n");
                observer_mananger->register_observee(this);
            }
        }
    }

    if (create_history_directory())
    {
        if (get_progress(&_progress) && open_progress())
        {
            _signal_thread = new sys::CThreadEngine(sys::bind(&CDbProcess::signal_thread, this));
            while (!_stop_signal_thread)
            {
                delete_old_history_files();

                if (parent_process_not_exists())
                {
                    break;
                }
                if (!_db_connected && !connect_db())
                {
                    sys::CUtils::millisleep(1000);
                    continue;
                }

                handle_directory();
            }
        }
    }

    MYLOG_INFO("dbprocess(%u, %s) exit now\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str());
}

void CDbProcess::signal_thread()
{
    while (!_stop_signal_thread)
    {
        sys::CSignalHandler::handle(this);
    }
}

void CDbProcess::on_terminated()
{
    _stop_signal_thread = true;
    MYLOG_INFO("dbprocess(%u, %s) will exit\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str());
    if (_report_frequency_seconds > 0)
        mooon::observer::destroy();
}

void CDbProcess::on_child_end(pid_t child_pid, int child_exited_status)
{
    MYLOG_INFO("dbprocess(%u, %s) receive SIGCHLD with status: %d\n", static_cast<unsigned int>(child_pid), _dbinfo.str().c_str(), child_exited_status);
}

void CDbProcess::on_signal_handler(int signo)
{
    MYLOG_INFO("db process(%u, %s) receive signal: (%d)%s\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), signo, strsignal(signo));
}

void CDbProcess::on_exception(int errcode) throw ()
{
    MYLOG_ERROR("dbprocess(%u, %s) error: (%d)%s\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), errcode, sys::Error::to_string(errcode).c_str());
}

bool CDbProcess::is_over(uint32_t offset) const
{
    return offset >= static_cast<uint32_t>(argument::sql_file_size->value());
}

bool CDbProcess::parent_process_not_exists() const
{
    if (0 == mooon::argument::auto_exit->value())
    {
        // 不自动退出
        return false;
    }
    else
    {
        pid_t ppid = getppid();
        if (1 == ppid)
        {
            // 父进程不在则自动退出
            MYLOG_INFO("dbprocess(%u, %s) will exit for parent process not exit\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str());
            return true;
        }

        return false;
    }
}

bool CDbProcess::create_history_directory() const
{
    const std::string history_directory = _log_dirpath + std::string("/history");

    try
    {
        //sys::CDirUtils::create_directory(history_directory.c_str(), DIRECTORY_DEFAULT_PERM);
        sys::CDirUtils::create_directory_recursive(history_directory.c_str(), DIRECTORY_DEFAULT_PERM);
        MYLOG_INFO("create directory ok: %s\n", history_directory.c_str());
        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        if (EEXIST == ex.errcode())
            return true;

        MYLOG_ERROR("create directory[%s] failed: %s\n", history_directory.c_str(), sys::Error::to_string().c_str());
        return false;
    }
}

void CDbProcess::handle_directory()
{
    std::vector<std::string>* subdir_names = NULL;
    std::vector<std::string>* link_names = NULL;
    std::vector<std::string> file_names;

    sys::CDirUtils::list(_log_dirpath, subdir_names, &file_names, link_names);
    if (file_names.empty())
    {
        if (!_stop_signal_thread)
        {
            sys::CUtils::millisleep(1000);
        }
    }
    else
    {
        // 文件名格式为：
        // sql.timestamp.suffix，需要按timespamp从小到大排充，如果timestamp相同则按suffix从小到大排序
        std::sort(file_names.begin(), file_names.end());

        for (std::vector<std::string>::size_type i=0; !_stop_signal_thread&&i<file_names.size(); ++i)
        {
            const std::string& filename = file_names[i];

            if (parent_process_not_exists())
            {
                break;
            }
            if (is_sql_log_filename(filename))
            {
                if (file_handled(filename))
                    archive_file(filename);
                else
                    handle_file(filename);
            }
            if (_stop_signal_thread)
            {
                break;
            }
        }
    }
}

bool CDbProcess::handle_file(const std::string& filename)
{
    uint32_t offset = 0;
    const std::string& log_tag = _dbinfo.alias + std::string("/") + filename;
    const std::string& log_filepath = _log_dirpath + std::string("/") + filename;

    MYLOG_INFO("handling: %s\n", log_filepath.c_str());
    int fd = open(log_filepath.c_str(), O_RDONLY);
    if (-1 == fd)
    {
        MYLOG_ERROR("open %s error: %s\n", log_filepath.c_str(), sys::Error::to_string().c_str());
        return false;
    }

    sys::CloseHelper<int> close_helper(fd);
    if (is_current_file(filename))
    {
        if (-1 == lseek(fd, _progress.offset, SEEK_SET))
        {
            MYLOG_ERROR("lseek %s error(%s): %s\n", _progress.str().c_str(), log_filepath.c_str(), sys::Error::to_string().c_str());
            return false;
        }
        else
        {
            offset = _progress.offset;
            MYLOG_INFO("lseek %s to %u offset: %s\n", _progress.str().c_str(), _progress.offset, log_filepath.c_str());
        }
    }

    int count = 0; // 入库的条数
    int consecutive_nodata = 0; // 连续无data的次数
    while (!_stop_signal_thread)
    {
        int bytes = 0;
        int32_t length = 0;
        if (parent_process_not_exists())
        {
            break;
        }

        // 读取SQL语句长度
        bytes = read(fd, &length, sizeof(length));
        if (-1 == bytes)
        {
            MYLOG_ERROR("read %s:%u error: %s\n", log_filepath.c_str(), offset, sys::Error::to_string().c_str());
            return false;
        }
        else if (0 == bytes)
        {
            if (is_over(offset))
            {
                archive_file(filename);
                break;
            }
            if (0 == consecutive_nodata++%1000)
            {
                MYLOG_INFO("[%s:%u]no data to sleep: %s\n", log_filepath.c_str(), offset, _progress.str().c_str());
            }

            // 可以考虑引入inotify，改轮询为监听方式
            sys::CUtils::millisleep(1000);
            continue;
        }

        consecutive_nodata = 0; // reset
        if (bytes > 0)
        {
            offset += static_cast<uint32_t>(bytes);
        }

        // 读取SQL语句
        std::string sql(length, '\0');
        bytes = read(fd, const_cast<char*>(sql.data()), length);
        if (-1 == bytes)
        {
            MYLOG_ERROR("read %s:%u error: %s\n", log_filepath.c_str(), offset, sys::Error::to_string().c_str());
            return false;
        }
        if (bytes > 0)
        {
            offset += static_cast<uint32_t>(bytes);
        }

        // 操作DB异常时不断重试
        MYLOG_DEBUG("%s\n", sql.c_str());
        while (!_stop_signal_thread)
        {
            if (parent_process_not_exists())
            {
                break;
            }

            try
            {
                const int rows = _mysql.update("%s", sql.c_str());
                const time_t end_time = time(NULL);
                const int interval = static_cast<int>(end_time - _begin_time);

                if (mooon::argument::batch->value() <= 1)
                {
                    // 非批量提交
                    ++count;
                    ++_interval_count;

                    if (interval >= mooon::argument::efficiency->value())
                    {
                        MYLOG_INFO("[%s:%u]efficiency: %d (%d, %ds)\n", log_tag.c_str(), offset, _interval_count/interval, _interval_count, interval);
                        _begin_time = end_time;
                        _interval_count = 0;
                    }
                }
                else
                {
                    ++_batch;

                    // 批量提交
                    if (_batch >= mooon::argument::batch->value() || (interval > 1))
                    {
                        _mysql.commit();
                        count += _batch;
                        _interval_count += _batch;
                        _batch = 0;

                        if (interval >= mooon::argument::efficiency->value())
                        {
                            MYLOG_INFO("[%s:%u] efficiency: %d (%d, %ds)\n", log_tag.c_str(), offset, _interval_count/interval, _interval_count, interval);
                            _begin_time = end_time;
                            _interval_count = 0;
                        }
                    }
                }

                // 更新进度
                if (!update_progress(filename, offset))
                {
                    return false;
                }

                // rows为0可能是失败，比如update时没有满足where条件的记录存在时
                if (0 == rows)
                {
                    MYLOG_WARN("[UPDATE_WARNING][%s:%u][%s] ok: %d, %" PRIu64"\n", log_tag.c_str(), offset, sql.c_str(), rows, _success_num_sqls);
                }
                else if (0 == ++_success_num_sqls%1000)
                {
                    MYLOG_INFO("[%s:%u][%s][ROWS:%d] %s-lines: %" PRIu64"\n", log_tag.c_str(), offset, sql.c_str(), rows, _dbinfo.alias.c_str(), _success_num_sqls);
                }
                else
                {
                    MYLOG_DEBUG("[%s:%u][%s] ok: %d, %" PRIu64"\n", log_tag.c_str(), offset, sql.c_str(),  rows, _success_num_sqls);
                }

                break;
            }
            catch (sys::CDBException& ex)
            {
                MYLOG_ERROR("[%s:%u]%s\n", log_tag.c_str(), offset, ex.str().c_str());

                // 网络类需要重试，直到成功
                if (!_mysql.is_disconnected_exception(ex))
                {
                    ++_failure_num_sqls;
                    break;
                }

                ++_retry_times;
                sys::CUtils::millisleep(1000);
            }
        }
    }

    return true;
}

bool CDbProcess::file_handled(const std::string& filename) const
{
    // 是否为已处理过的文件，如果是则跳过
    return (!_progress.empty()) && (-1 == strcmp(filename.c_str(), _progress.filename));
}

bool CDbProcess::is_current_file(const std::string& filename) const
{
    return 0 == strcmp(filename.c_str(), _progress.filename);
}

bool CDbProcess::connect_db()
{
    try
    {
        _mysql.set_host(_dbinfo.host, static_cast<uint16_t>(_dbinfo.port));
        _mysql.set_user(_dbinfo.user, _dbinfo.password);
        _mysql.set_db_name(_dbinfo.name);
        _mysql.set_charset(_dbinfo.charset);
        _mysql.enable_auto_reconnect();
        _mysql.open();

        // 如果批量提交则需要禁用自动提交
        if (mooon::argument::batch->value() > 1)
        {
            try
            {
                _mysql.enable_autocommit(false);
            }
            catch (sys::CDBException& ex)
            {
                _mysql.close();
                throw;
            }
        }

        _db_connected = true;
        _consecutive_failures = 0;
        MYLOG_INFO("dbprocess(%u, %s) connect %s ok\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), _mysql.str().c_str());
        return true;
    }
    catch (sys::CDBException& ex)
    {
        _db_connected = false;
        if (0 == _consecutive_failures++%60)
        {
            MYLOG_ERROR("dbprocess(%u, %s) connect %s error(%u): %s\n", static_cast<unsigned int>(getpid()), _dbinfo.str().c_str(), _mysql.str().c_str(), _consecutive_failures, ex.str().c_str());
        }

        return false;
    }
}

void CDbProcess::close_db()
{
    if (_db_connected)
    {
        _mysql.close();
        _db_connected = false;
    }
}

bool CDbProcess::update_progress(const std::string& filename, uint32_t offset)
{
    strncpy(_progress.filename, filename.c_str(), sizeof(_progress.filename));
    _progress.offset = offset;
    _progress.crc32 = _progress.get_crc32(); // get_crc32()依赖offset和filename，所以顺序不能颠倒

    ssize_t bytes_written = pwrite(_progess_fd, &_progress, sizeof(_progress), 0);
    if (bytes_written != static_cast<ssize_t>(sizeof(_progress)))
    {
        MYLOG_ERROR("write progess(%s:%d) error: %s\n", filename.c_str(), offset, sys::Error::to_string().c_str());
        return false;
    }

    MYLOG_DEBUG("update %s ok\n", _progress.str().c_str());
    return true;
}

bool CDbProcess::get_progress(struct Progress* progress)
{
    const std::string progress_filepath = _log_dirpath + std::string("/sql.progress");
    int progess_fd = open(progress_filepath.c_str(), O_RDONLY);

    if (-1 == progess_fd)
    {
        int errcode = sys::Error::code();
        if (errcode != ENOENT)
        {
            MYLOG_ERROR("open %s failed: (%d)%s\n", progress_filepath.c_str(), errcode, sys::Error::to_string(errcode).c_str());
            return false;
        }

        return true;
    }
    else
    {
        sys::CloseHelper<int> close_helper(progess_fd);

        MYLOG_INFO("open %s ok\n", progress_filepath.c_str());
        ssize_t bytes_read = pread(progess_fd, progress, sizeof(*progress), 0);
        if (0 == bytes_read)
        {
            // 空文件
            MYLOG_INFO("progess is empty\n");
            return true;
        }
        else if (bytes_read != sizeof(*progress))
        {
            // 被损坏的文件
            MYLOG_ERROR("get progress(%s) failed: %zd,%zd\n", progress_filepath.c_str(), bytes_read, sizeof(*progress));
            return false;
        }
        else
        {
            uint32_t crc32 = progress->get_crc32();
            if (crc32 != progress->crc32)
            {
                MYLOG_ERROR("crc32 progress(%s) failed: (%u)%s\n", progress_filepath.c_str(), crc32, progress->str().c_str());
                return false;
            }

            MYLOG_INFO("get %s ok\n", progress->str().c_str());
            return true;
        }
    }
}

bool CDbProcess::open_progress()
{
    const std::string progress_filepath = _log_dirpath + std::string("/sql.progress");
    int progess_fd = open(progress_filepath.c_str(), O_WRONLY|O_CREAT, FILE_DEFAULT_PERM);
    if (-1 == progess_fd)
    {
        int errcode = sys::Error::code();
        MYLOG_ERROR("open %s failed: (%d)%s\n", progress_filepath.c_str(), errcode, sys::Error::to_string(errcode).c_str());
        return false;
    }

    _progess_fd = progess_fd;
    return true;
}

void CDbProcess::archive_file(const std::string& filename) const
{
    const std::string& filepath = get_filepath(filename);
    const std::string& archived_filepath = get_archived_filepath(filename);
    if (-1 == rename(filepath.c_str(), archived_filepath.c_str()))
    {
        MYLOG_ERROR("archived %s to %s failed: %s\n", filepath.c_str(), archived_filepath.c_str(), sys::Error::to_string().c_str());
    }
    else
    {
        MYLOG_INFO("archived %s to %s ok\n", filepath.c_str(), archived_filepath.c_str());
    }
}

std::string CDbProcess::get_filepath(const std::string& filename) const
{
    return _log_dirpath + std::string("/") + filename;
}

std::string CDbProcess::get_archived_filepath(const std::string& filename) const
{
    const std::string& history_dirpath = get_history_dirpath();
    return history_dirpath + std::string("/") + filename;
}

std::string CDbProcess::get_history_dirpath() const
{
    return _log_dirpath + std::string("/history");
}

void CDbProcess::delete_old_history_files()
{
    const time_t current_seconds = time(NULL);
    struct tm current_struct;
    (void)localtime_r(&current_seconds, &current_struct);

    if (current_struct.tm_hour != mooon::argument::history_hour->value())
    {
        _old_history_files_deleted_today = false;
    }
    else
    {
        if (!_old_history_files_deleted_today)
        {
            _old_history_files_deleted_today = true;

            std::vector<std::string> file_names;
            std::vector<std::string>* subdir_names = NULL;
            std::vector<std::string>* link_names= NULL;
            const std::string& history_dirpath = get_history_dirpath();

            mooon::sys::CDirUtils::list(history_dirpath, subdir_names, &file_names, link_names);
            for (std::vector<std::string>::size_type i=0; !_stop_signal_thread&&i<file_names.size(); ++i)
            {
                struct stat st;
                const std::string& filename = file_names[i];
                const std::string& filepath = history_dirpath + std::string("/") + filename;

                if (-1 == stat(filepath.c_str(), &st))
                {
                    MYLOG_ERROR("stat %s error: %s\n", filepath.c_str(), sys::Error::to_string().c_str());
                }
                else
                {
                    const int64_t interval_seconds = static_cast<int64_t>(current_seconds - st.st_mtime);

                    if (interval_seconds < 3600*24*mooon::argument::history_days->value())
                    {
                        MYLOG_DEBUG("keep %s: %" PRId64", %" PRId64", %" PRId64"\n", filepath.c_str(), interval_seconds, (int64_t)current_seconds, (int64_t)st.st_mtime);
                    }
                    else
                    {
                        try
                        {
                            MYLOG_INFO("to remove history sql log(%" PRId64", %" PRId64", %" PRId64"): %s\n", interval_seconds, (int64_t)current_seconds, (int64_t)st.st_mtime, filepath.c_str());
                            mooon::sys::CFileUtils::remove(filepath.c_str());
                        }
                        catch (mooon::sys::CSyscallException& ex)
                        {
                            MYLOG_ERROR("remove %s failed: %s\n", filepath.c_str(), ex.str().c_str());
                        }
                    }
                }
            }
        }
    }
}

void CDbProcess::on_report(mooon::observer::IDataReporter* data_reporter, const std::string& current_datetime)
{
    if (((_success_num_sqls > 0) && (_success_num_sqls > _last_success_num_sqls)) ||
        ((_failure_num_sqls > 0) && (_failure_num_sqls > _last_failure_num_sqls)) ||
        ((_retry_times > 0) && (_retry_times > _last_retry_times)))
    {
        _last_success_num_sqls = _success_num_sqls;
        _last_failure_num_sqls = _failure_num_sqls;
        _last_retry_times = _retry_times;

        data_reporter->report("[%s]%" PRIu64",%" PRIu64",%" PRIu64"\n", current_datetime.c_str(), _success_num_sqls, _failure_num_sqls, _retry_times);
    }
}

void CDbProcess::reset()
{
    _success_num_sqls = 0;
    _last_success_num_sqls = 0;
    _failure_num_sqls = 0;
    _last_failure_num_sqls = 0;
    _retry_times = 0;
    _last_retry_times = 0;
}

}} // namespace mooon { namespace db_proxy {
