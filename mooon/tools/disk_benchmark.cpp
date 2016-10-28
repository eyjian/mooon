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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
// 磁盘性能测试工具，用于测试磁盘的读写性能：每秒写和读的笔数、每笔写和读花费的时长
// 可以指定每次写和读的块大小，以及连读多少次写和读来求平均值。
#include <mooon/sys/close_helper.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/print_color.h>
#include <mooon/utils/string_utils.h>
#include <signal.h>

STRING_ARG_DEFINE(dir, ".", "temporary file directory");
INTEGER_ARG_DEFINE(uint32_t, block, 4096, 1, 1024*1024*1024, "block bytes");
INTEGER_ARG_DEFINE(uint32_t, times, 100000, 1, std::numeric_limits<uint32_t>::max(), "times to write and read");
INTEGER_ARG_DEFINE(uint8_t, buffer, 1, 0, 1, "buffer write");

static char filename[sizeof("disk_benchmark_XXXXXX")] = { '\0' };
static void onsignal(int signo)
{
    if (SIGINT == signo)
    {
        if (filename[0] != '\0')
        {
            remove(filename);
            exit(1);
        }
    }
}

int main(int argc, char* argv[])
{
    fprintf(stdout, "please run under DIRECTORY: " PRINT_COLOR_YELLOW"%s" PRINT_COLOR_NONE"\n", mooon::sys::CUtils::get_program_path().c_str());

    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }

    signal(SIGINT, onsignal);
    strncpy(filename, "disk_benchmark_XXXXXX", sizeof("disk_benchmark_XXXXXX")-1);
    int fd = mkstemp(filename);
    if (-1 == fd)
    {
        fprintf(stderr, "open %s error: %m\n", filename);
        remove(filename);
        exit(1);
    }

    fprintf(stdout, "block size: %u, file: %s\n\n", mooon::argument::block->value(), filename);
    const std::string buffer(mooon::argument::block->value(), '#');
    mooon::sys::CStopWatch stop_watch;
    int64_t total_bytes = 0;
    ssize_t bytes = 0;
    for (uint32_t i=0; i<mooon::argument::times->value(); ++i)
    {
        bytes = write(fd, buffer.data(), buffer.size());
        if (bytes != static_cast<ssize_t>(buffer.size()))
        {
            fprintf(stderr, "write %s error: %m\n", filename);
            remove(filename);
            close(fd);
            exit(1);
        }

        if (0 == mooon::argument::buffer->value())
        {
            if (-1 == fsync(fd))
            {
                fprintf(stderr, "fsync %s error: %m\n", filename);
                remove(filename);
                close(fd);
                exit(1);
            }
        }

        total_bytes += bytes;
    }

    unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
    unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
    unsigned int elapsed_seconds = elapsed_microseconds / 1000000;
    unsigned int iops = (0 == elapsed_seconds)? mooon::argument::times->value(): mooon::argument::times->value()/elapsed_seconds;
    double rate =  elapsed_microseconds / mooon::argument::times->value();
    double total_bytes_k = total_bytes / 1024;
    double total_bytes_m = total_bytes_k / 1024;
    double bytes_sec = (0 == elapsed_seconds)? total_bytes_m: total_bytes_m / elapsed_seconds;
    fprintf(stdout, "[" PRINT_COLOR_YELLOW"WRITE" PRINT_COLOR_NONE"]\n");
    fprintf(stdout, "bytes: %" PRId64" (%.02fMB, %.02fMB/s), times: %u\n", total_bytes, total_bytes_m, bytes_sec, mooon::argument::times->value());
    fprintf(stdout, "microseconds: %u, milliseconds: %u, seconds: %u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
    fprintf(stdout, "iops: %u, rate: %0.2fus (%0.2fms)\n", iops, rate, rate/1000);

    close(fd);
    fprintf(stdout, "\n" PRINT_COLOR_GREEN);
    fprintf(stdout, "free pagecache: echo 1 > /proc/sys/vm/drop_caches\n");
    fprintf(stdout, "free dentries and inodes: echo 2 > /proc/sys/vm/drop_caches\n");
    fprintf(stdout, "free pagecache, dentries and inodes: echo 3 > /proc/sys/vm/drop_caches\n");
    fprintf(stdout, "press ENTER to continue ...\n" PRINT_COLOR_NONE);
    getchar();

    fd = open(filename, O_RDONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "open %s error: %m\n", filename);
        remove(filename);
        exit(1);
    }

    total_bytes = 0;
    stop_watch.restart();
    for (uint32_t i=0; i<mooon::argument::times->value(); ++i)
    {
        bytes = read(fd, const_cast<char*>(buffer.data()), mooon::argument::block->value());
        if (bytes != static_cast<ssize_t>(buffer.size()))
        {
            fprintf(stderr, "read %s error: %m\n", filename);
            remove(filename);
            close(fd);
            exit(1);
        }

        total_bytes += bytes;
    }

    elapsed_microseconds = stop_watch.get_elapsed_microseconds();
    elapsed_milliseconds = elapsed_microseconds / 1000;
    elapsed_seconds = elapsed_microseconds / 1000000;
    iops = (0 == elapsed_seconds)? mooon::argument::times->value(): mooon::argument::times->value()/elapsed_seconds;
    rate =  elapsed_microseconds / mooon::argument::times->value();
    total_bytes_k = total_bytes / 1024;
    total_bytes_m = total_bytes_k / 1024;
    bytes_sec = (0 == elapsed_seconds)? total_bytes_m: total_bytes_m / elapsed_seconds;
    fprintf(stdout, "[" PRINT_COLOR_YELLOW"READ" PRINT_COLOR_NONE"]\n");
    fprintf(stdout, "bytes: %" PRId64" (%.02fMB/s), times: %u\n", total_bytes, bytes_sec, mooon::argument::times->value());
    fprintf(stdout, "microseconds: %u, milliseconds: %u, seconds: %u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
    fprintf(stdout, "iops: %u, rate: %0.2fus (%0.2fms)\n", iops, rate, rate/1000);

    remove(filename);
    close(fd);
    return 0;
}
