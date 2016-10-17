// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "config_loader.h"
#include <mooon/net/thrift_helper.h>
#include <mooon/observer/observer_manager.h>
#include <mooon/sys/close_helper.h>
#include <mooon/sys/dir_utils.h>
#include <mooon/sys/main_template.h>
#include <mooon/sys/mysql_db.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/scoped_ptr.h>

// sqllog directory or file
STRING_ARG_DEFINE(sqllog, ".", "sqllog file or directory");
// 添加文件完整标志
INTEGER_ARG_DEFINE(uint8_t, add_endtag, 0, 0, 1, "add endtag for sqllog file");

static int check_file(const char* filepath);
static int check_directory(const char* dirpath);
static bool write_endtag(const char* filepath);

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        return false;
    }

    try
    {
        if (mooon::sys::CUtils::is_file(mooon::argument::sqllog->c_value()))
            return check_file(mooon::argument::sqllog->c_value());
        else
            return check_directory(mooon::argument::sqllog->c_value());
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }
}

int check_file(const char* filepath)
{
    const std::string filename = mooon::utils::CStringUtils::extract_filename(filepath);
    if (!mooon::db_proxy::is_sql_log_filename(filename))
    {
        fprintf(stderr, "[NOTSQLLOG] %s\n", filepath);
        return 1;
    }

    int fd = open(filepath, O_RDONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "open %s failed: %m\n", filepath);
        return 1;
    }

    mooon::sys::CloseHelper<int> close_helper(fd);
    while (true)
    {
        int32_t length = 0;
        int bytes = read(fd, &length, sizeof(length));
        if (0 == bytes)
        {
            fprintf(stdout, "[NOTEND] %s\n", filepath);
            if (1 == mooon::argument::add_endtag->value())
            {
                if (write_endtag(filepath))
                {
                    fprintf(stdout, "add endtag ok: %s\n", filepath);
                }
                else
                {
                    fprintf(stderr, "[%s]: add endtag failed: %m\n", filepath);
                }
            }

            break;
        }
        if (bytes != sizeof(length))
        {
            fprintf(stderr, "read %s length error: %d\n", filepath, bytes);
            return 1;
        }
        if (0 == length)
        {
            fprintf(stdout, "[END] %s\n", filepath);
            break;
        }
        else
        {
            std::string sql(length, '\0');
            bytes = read(fd, const_cast<char*>(sql.data()), length);
            if (0 == bytes)
            {
                fprintf(stderr, "[BAD] %s\n", filepath);
                return 1;
            }
        }
    }

    return 0;
}

int check_directory(const char* dirpath)
{
    int num_error = 0;
    std::vector<std::string>* subdir_names = NULL;
    std::vector<std::string> file_names;
    std::vector<std::string>* link_names = NULL;
    mooon::sys::CDirUtils::list(dirpath, subdir_names, &file_names, link_names);

    for (std::vector<std::string>::size_type i=0; i<file_names.size(); ++i)
    {
        if (mooon::db_proxy::is_sql_log_filename(file_names[i]))
        {
            const std::string filepath = mooon::utils::CStringUtils::format_string("%s/%s", dirpath, file_names[i].c_str());
            if (check_file(filepath.c_str()) != 0)
                ++num_error;
        }
    }

    return 0 == num_error;
}

bool write_endtag(const char* filepath)
{
    int fd = open(filepath, O_WRONLY|O_APPEND);
    if (-1 == fd)
    {
        fprintf(stderr, "open %s failed: %m\n", filepath);
        return false;
    }

    mooon::sys::CloseHelper<int> close_helper(fd);
    int32_t length = 0;
    return write(fd, &length, sizeof(length)) == sizeof(length);
}
