// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "sql_progress.h"
#include <mooon/utils/args_parser.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
STRING_ARG_DEFINE(progress, "", "sql.progress file path, e.g., --progress=/home/db_proxy/sqllog_4077/test/sql.progress");

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }

    if (!mooon::argument::progress->value().empty())
    {
        const int fd = open(mooon::argument::progress->c_value(), O_RDONLY);
        if (-1 == fd)
        {
            fprintf(stderr, "open %s error: %m\n", mooon::argument::progress->c_value());
            exit(1);
        }

        struct mooon::db_proxy::Progress progress;
        if (read(fd, &progress, sizeof(progress)) != sizeof(progress))
        {
            fprintf(stderr, "read %s error: %m\n", mooon::argument::progress->c_value());
            close(fd);
            exit(1);
        }

        close(fd);
        fprintf(stdout, "%s\n", progress.str().c_str());
    }
    else
    {
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }

    return 0;
}
