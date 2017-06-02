// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "db_process.h"
#include <algorithm>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <string.h>
#include <sys/uio.h>

INTEGER_ARG_DECLARE(uint16_t, port);
INTEGER_ARG_DECLARE(int, sql_file_size);
INTEGER_ARG_DECLARE(uint8_t, batch);
INTEGER_ARG_DECLARE(uint16_t, efficiency);
namespace mooon { namespace db_proxy {

CDbProcess::CDbProcess(const struct DbInfo& dbinfo)
    : _progess_fd(-1), _dbinfo(dbinfo), _stop_signal_thread(false), _signal_thread(NULL),
      _consecutive_failures(0), _num_sqls(0), _db_connected(false)
{
    const std::string program_path = sys::CUtils::get_program_path();
    _log_dirpath = get_log_dirpath(_dbinfo.alias);

    _begin_time = time(NULL);
    _interval_count = 0;
    _batch = 0;
}

CDbProcess::~CDbProcess()
{
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
    MYLOG_INFO("db_process(%u): %s\n", getpid(), db_process_title.c_str());

    if (create_history_directory())
    {
        if (get_progress(&_progress) && open_progress())
        {
            _signal_thread = new sys::CThreadEngine(sys::bind(&CDbProcess::signal_thread, this));
            while (!_stop_signal_thread)
            {
                if (parent_process_not_exists())
                    break;

                if (!_db_connected && !connect_db())
                {
                    sys::CUtils::millisleep(1000);
                    continue;
                }

                handle_directory();
                if (!_stop_signal_thread)
                    sys::CUtils::millisleep(1000);
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

bool CDbProcess::parent_process_not_exists() const
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
    if (!file_names.empty())
    {
        // 文件名格式为：
        // sql.timestamp.suffix，需要按timespamp从小到大排充，如果timestamp相同则按suffix从小到大排序
        std::sort(file_names.begin(), file_names.end());

        for (std::vector<std::string>::size_type i=0; !_stop_signal_thread&&i<file_names.size(); ++i)
        {
            if (parent_process_not_exists())
                break;

            const std::string& filename = file_names[i];
            if (is_sql_log_filename(filename))
                handle_file(filename);
            if (_stop_signal_thread)
                break;
        }
    }
}

bool CDbProcess::handle_file(const std::string& filename)
{
    if (file_handled(filename))
    {
        MYLOG_INFO("handled: %s\n", filename.c_str());
    }
    else
    {
        MYLOG_DEBUG("handling: %s\n", filename.c_str());
    }

    const std::string log_filepath = _log_dirpath + std::string("/") + filename;
    uint32_t offset = 0;
    int fd = open(log_filepath.c_str(), O_RDONLY);
    if (-1 == fd)
    {
        MYLOG_ERROR("open %s error: %s\n", log_filepath.c_str(), sys::Error::to_string().c_str());
        return false;
    }
    else
    {
        sys::CloseHelper<int> close_helper(fd);
        if (is_current_file(filename))
        {
            if (-1 == lseek(fd, _progress.offset, SEEK_SET))
            {
                MYLOG_ERROR("lseek %s error: %s\n", _progress.str().c_str(), sys::Error::to_string().c_str());
                return false;
            }

            offset = _progress.offset;
            MYLOG_INFO("lseek %s to %u\n", _progress.str().c_str(), _progress.offset);
        }

        int count = 0; // 入库的条数
        int consecutive_nodata = 0; // 连续无data的次数
        while (!_stop_signal_thread)
        {
            int bytes = 0;
            int32_t length = 0;
            if (parent_process_not_exists())
                break;

            // 读取SQL语句长度
            bytes = read(fd, &length, sizeof(length));
            if (-1 == bytes)
            {
                MYLOG_ERROR("read %s error: %s\n", log_filepath.c_str(), sys::Error::to_string().c_str());
                return false;
            }
            else if (0 == bytes)
            {
                if (0 == consecutive_nodata++%1000)
                {
                    MYLOG_INFO("no data to sleep: %s\n", _progress.str().c_str());
                }

                // 可以考虑引入inotify，改轮询为监听方式
                sys::CUtils::millisleep(1000);
                continue;
            }
            if (0 == length)
            {
                // END
                MYLOG_INFO("%s ENDED: %d\n", _progress.str().c_str(), count);
                // 归档
                archive_file(filename);
                break;
            }

            consecutive_nodata = 0; // reset
            if (bytes > 0)
                offset += static_cast<uint32_t>(bytes);

            // 读取SQL语句
            std::string sql(length, '\0');
            bytes = read(fd, const_cast<char*>(sql.data()), length);
            if (-1 == bytes)
            {
                MYLOG_ERROR("read %s error: %s\n", log_filepath.c_str(), sys::Error::to_string().c_str());
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
                    break;

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
                            MYLOG_INFO("efficiency: %d (%d, %ds)\n", _interval_count/interval, _interval_count, interval);
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
                                MYLOG_INFO("efficiency: %d (%d, %ds)\n", _interval_count/interval, _interval_count, interval);
                                _begin_time = end_time;
                                _interval_count = 0;
                            }
                        }
                    }

                    // 更新进度
                    if (!update_progress(filename, offset))
                        return false;

                    // rows为0可能是失败，比如update时没有满足where条件的记录存在时
                    if (0 == rows)
                    {
                        MYLOG_WARN("[UPDATE_WARNING][%s] ok: %d, %" PRIu64"\n", sql.c_str(), rows, _num_sqls);
                    }
                    else if (0 == ++_num_sqls%10000)
                    {
                        MYLOG_INFO("[%s] ok: %d, %" PRIu64"\n", sql.c_str(), rows, _num_sqls);
                    }
                    else
                    {
                        MYLOG_DEBUG("[%s] ok: %d, %" PRIu64"\n", sql.c_str(), rows, _num_sqls);
                    }

                    break;
                }
                catch (sys::CDBException& ex)
                {
                    MYLOG_ERROR("%s\n", ex.str().c_str());

                    // 网络类需要重试，直到成功
                    if (!_mysql.is_disconnected_exception(ex))
                        break;
                    sys::CUtils::millisleep(1000);
                }
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
    const std::string filepath = _log_dirpath + std::string("/") + filename;
    const std::string archived_filepath = _log_dirpath + std::string("/history/") + filename;
    if (-1 == rename(filepath.c_str(), archived_filepath.c_str()))
    {
        MYLOG_ERROR("archived %s to %s failed: %s\n", filepath.c_str(), archived_filepath.c_str(), sys::Error::to_string().c_str());
    }
    else
    {
        MYLOG_INFO("archived %s to %s ok\n", filepath.c_str(), archived_filepath.c_str());
    }
}

}} // namespace mooon { namespace db_proxy {
