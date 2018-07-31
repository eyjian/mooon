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
 *
 * 搬移redis队列工具，用于将redis一个队列的数据复制到另一队列中，要求为左进右出
 */
#include <r3c/r3c.h>
#include <mooon/sys/atomic.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>

// 如何确定一个redis的key？
// 通过前缀prefix加序号的方式，其它前缀prefix由参数指定，但可以为空，
// 序号从0开始按1递增，最大值为参数queues的值减1
//
// 假设前缀prefix值为“mooon:”，queues值为3，则有如下三个key：
// mooon:0
// mooon:1
// mooon:2

// 队列数
// 源队列和目标队列数均由此参数决定，也即源队列和目标队列的个数是相等的
INTEGER_ARG_DEFINE(uint8_t, queues, 1, 1, 10, "the number of queues, e.g. --queues=1");

// 线程数系数，
// 注意并不是线程数，线程数为：threads * queues，
// 假设queues参数值为2，threads参数值为3，则线程数为6
INTEGER_ARG_DEFINE(uint8_t, threads, 1, 1, 18, "(threads * queues) to get number of move threads, e.g., --threads=1");

// 源redis
STRING_ARG_DEFINE(src_redis, "", "the nodes of source redis, e.g., --src_redis=127.0.0.1:6379,127.0.0.1:6380");

// 目标redis
// 当源和目标相同时，应当指定不同的prefix，虽然也可以都相同，但那样无实际意义了
STRING_ARG_DEFINE(dst_redis, "", "the nodes of destination redis, e.g., --dst_redis=127.0.0.1:6381,127.0.0.1:6382");

// 源队列Key前缀
STRING_ARG_DEFINE(src_prefix, "", "the key prefix of source queue, e.g., --src_prefix='mooon:'");

// 目标队列Key前缀
STRING_ARG_DEFINE(dst_prefix, "", "the key prefix of destination queue, e.g., --src_prefix='mooon:'");

// 源队列Key是否仅由前缀组成，即src_prefix是key，或只是key的前缀
INTEGER_ARG_DEFINE(uint8_t, src_only_prefix, 0, 0, 1, "the prefix is the key of source");

// 目标队列Key是否仅由前缀组成，即dst_prefix是key，或只是key的前缀
INTEGER_ARG_DEFINE(uint8_t, dst_only_prefix, 0, 0, 1, "the prefix is the key of destination");

// 多少个时输出一次计数
INTEGER_ARG_DEFINE(uint32_t, tick, 10000, 1, 10000000, "the times to tick");

// 统计频率（单位：秒）
INTEGER_ARG_DEFINE(uint32_t, stat_interval, 2, 1, 86400, "the interval to stat in seconds");

// 轮询队列和重试操作的间隔（单位为毫秒）
INTEGER_ARG_DEFINE(uint32_t, retry_interval, 100, 1, 1000000, "the interval in milliseconds to poll or retry");

// 批量数，即一次批量移动多少
INTEGER_ARG_DEFINE(int, batch, 1, 1, 100000, "batch to move");

static volatile bool g_stop = false;
#if __WORDSIZE==64
static atomic8_t g_num_moved;
#else
static atomic_t g_num_moved;
#endif

static void on_terminated();
static void signal_thread_proc(); // 信号线程
static void stat_thread_proc(); // 统计线程
static void move_thread_proc(uint8_t i); // 移动线程
static std::string get_src_key(uint8_t i);
static std::string get_dst_key(uint8_t i);

int main(int argc, char* argv[])
{
    std::string errmsg;

    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n\n", errmsg.c_str());
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    else if (mooon::argument::src_redis->value().empty())
    {
        fprintf(stderr, "parameter[--src_redis] not set\n\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    else if (mooon::argument::dst_redis->value().empty())
    {
        fprintf(stderr, "parameter[--dst_redis] not set\n\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    else if (mooon::argument::src_prefix->value().empty())
    {
        fprintf(stderr, "parameter[--src_prefix] not set\n\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    else if (mooon::argument::dst_prefix->value().empty())
    {
        fprintf(stderr, "parameter[--dst_prefix] not set\n\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }

    try
    {
#if __WORDSIZE==64
        atomic8_set(&g_num_moved, 0);
#else
        atomic_set(&g_num_moved, 0);
#endif

        mooon::sys::g_logger = mooon::sys::create_safe_logger();
        mooon::sys::CThreadEngine* signal_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&signal_thread_proc));
        mooon::sys::CThreadEngine* stat_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&stat_thread_proc));

        const uint8_t num_queues = mooon::argument::queues->value();
        const uint8_t num_threads = num_queues * mooon::argument::threads->value();
        mooon::sys::CThreadEngine** thread_engines = new mooon::sys::CThreadEngine*[num_threads];
        for (uint8_t i=0; i<num_threads; ++i)
        {
            thread_engines[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(&move_thread_proc, i));
        }
        for (uint8_t i=0; i<num_threads; ++i)
        {
            thread_engines[i]->join();
            delete thread_engines[i];
        }
        delete []thread_engines;

        stat_thread->join();
        delete stat_thread;
        signal_thread->join();
        delete signal_thread;
        MYLOG_INFO("mover exit\n");
        return 0;
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        MYLOG_ERROR("%s\n", ex.str().c_str());
        exit(1);
    }
}

void on_terminated()
{
    g_stop = true;
}

void signal_thread_proc()
{
    while (!g_stop)
    {
        mooon::sys::CSignalHandler::handle(&on_terminated, NULL, NULL, NULL);
    }
}

void stat_thread_proc()
{
    try
    {
        const uint32_t seconds = mooon::argument::stat_interval->value();
        uint64_t old_num_moved = 0;
        uint64_t last_num_moved = 0;
        mooon::sys::ILogger* stat_logger = mooon::sys::create_safe_logger(true, mooon::SIZE_32, "stat");

        stat_logger->enable_raw_log(true, true);
        while (!g_stop)
        {
            mooon::sys::CUtils::millisleep(seconds * 1000);
#if __WORDSIZE==64
            last_num_moved = atomic8_read(&g_num_moved);
#else
            last_num_moved = atomic_read(&g_num_moved);
#endif

            if (last_num_moved > old_num_moved)
            {
                const int num_moved = static_cast<int>((last_num_moved - old_num_moved) / seconds);
                stat_logger->log_raw(" %" PRId64" %" PRId64" %" PRId64" %d/s MOVED\n", last_num_moved, old_num_moved, last_num_moved - old_num_moved, num_moved);
            }

            old_num_moved = last_num_moved;
        }
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        MYLOG_ERROR("create stat logger failed: %s\n", ex.str().c_str());
    }
}

void move_thread_proc(uint8_t i)
{
    uint32_t num_moved = 0; // 已移动的数目
    uint32_t old_num_moved = 0; // 上一次移动的数目
    const uint32_t retry_interval = mooon::argument::retry_interval->value();
    const std::string& src_key = get_src_key(i);
    const std::string& dst_key = get_dst_key(i);
    r3c::CRedisClient src_redis(mooon::argument::src_redis->value());
    r3c::CRedisClient dst_redis(mooon::argument::dst_redis->value());
    std::vector<std::string> values;

    MYLOG_INFO("[%s] => [%s]\n", src_key.c_str(), dst_key.c_str());
    while (!g_stop)
    {
        values.clear();

        for (int k=0; !g_stop&&k<mooon::argument::batch->value(); ++k)
        {
            try
            {
                std::string value;

                if (!src_redis.rpop(src_key, &value))
                {
                    break;
                }
                else
                {
                    values.push_back(value);
                    MYLOG_DEBUG("[%d] %s\n", k, value.c_str());
                }
            }
            catch (r3c::CRedisException& ex)
            {
                MYLOG_ERROR("[%s]: %s\n", src_key.c_str(), ex.str().c_str());
            }
        }
        if (values.empty())
        {
            mooon::sys::CUtils::millisleep(retry_interval);
            continue;
        }

        while (!values.empty())
        {
            try
            {
                dst_redis.lpush(dst_key, values);

                const uint32_t num_moved_ = static_cast<uint32_t>(values.size());
#if __WORDSIZE==64
                atomic8_add(num_moved_, &g_num_moved);
#else
                atomic_add(num_moved_, &g_num_moved);
#endif

                num_moved += num_moved_;
                if (num_moved - old_num_moved >= mooon::argument::tick->value())
                {
                    old_num_moved = num_moved;
                    MYLOG_INFO("[%s]=>[%s]: %u\n", src_key.c_str(), dst_key.c_str(), num_moved);
                }
                break;
            }
            catch (r3c::CRedisException& ex)
            {
                MYLOG_ERROR("[%s]=>[%s]: %s\n", src_key.c_str(), dst_key.c_str(), ex.str().c_str());
                mooon::sys::CUtils::millisleep(retry_interval);
            }
        }
    }

    MYLOG_INFO("move thread %d exit\n", i);
}

std::string get_src_key(uint8_t i)
{
    if (1 == mooon::argument::src_only_prefix->value())
        return mooon::argument::src_prefix->value();
    else
        return mooon::utils::CStringUtils::format_string("%s%d", mooon::argument::src_prefix->c_value(), (int)i);
}

std::string get_dst_key(uint8_t i)
{
    if (1 == mooon::argument::dst_only_prefix->value())
        return mooon::argument::dst_prefix->value();
    else
        return mooon::utils::CStringUtils::format_string("%s%d", mooon::argument::dst_prefix->c_value(), (int)i);
}
