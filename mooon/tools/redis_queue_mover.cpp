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
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/signal_handler.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>

// 队列数
INTEGER_ARG_DEFINE(uint8_t, queues, 1, 1, 10, "the number of queues, e.g. --queues=1");
// 线程数系数，实际的线程为：threads * queues
INTEGER_ARG_DEFINE(uint8_t, threads, 1, 1, 20, "the thread number to move, e.g. --threads=1");

// 源redis
STRING_ARG_DEFINE(src_redis, "", "the nodes of source redis, e.g., --src_redis=127.0.0.1:6379,127.0.0.1:6380");

// 目标redis
STRING_ARG_DEFINE(dst_redis, "", "the nodes of destination redis, e.g., --dst_redis=127.0.0.1:6381,127.0.0.1:6382");

// 源队列Key前缀
STRING_ARG_DEFINE(src_prefix, "", "the key prefix of source queue");
// 目标队列Key前缀
STRING_ARG_DEFINE(dst_prefix, "", "the key prefix of destination queue");

// 源队列Key是否仅由前缀组成
INTEGER_ARG_DEFINE(uint8_t, src_only_prefix, 0, 0, 1, "the prefix is the key of source");

// 目标队列Key是否仅由前缀组成
INTEGER_ARG_DEFINE(uint8_t, dst_only_prefix, 0, 0, 1, "the prefix is the key of destination");

// 多少个时输出一次计数
INTEGER_ARG_DEFINE(uint32_t, tick, 10000, 1, 1000000000, "the times to tick");

// 轮询和重试间隔，单位为毫秒
INTEGER_ARG_DEFINE(uint32_t, interval, 100, 1, 1000000, "the interval (milliseconds) to poll or retry");

static volatile bool g_stop = false;
static void on_terminated();
static void signal_thread_proc();
static void move_thread_proc(uint8_t i);
static std::string get_src_key(uint8_t i);
static std::string get_dst_key(uint8_t i);

int main(int argc, char* argv[])
{
    std::string errmsg;

    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        return 1;
    }
    else if (mooon::argument::src_redis->value().empty())
    {
        fprintf(stderr, "parameter[--src_redis] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        return 1;
    }
    else if (mooon::argument::dst_redis->value().empty())
    {
        fprintf(stderr, "parameter[--dst_redis] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        return 1;
    }
    else if (mooon::argument::src_prefix->value().empty())
    {
        fprintf(stderr, "parameter[--src_prefix] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        return 1;
    }
    else if (mooon::argument::dst_prefix->value().empty())
    {
        fprintf(stderr, "parameter[--dst_prefix] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        return 1;
    }
    else
    {
        try
        {
            r3c::set_debug_log_write(NULL);
            r3c::set_info_log_write(NULL);
            r3c::set_error_log_write(NULL);

            mooon::sys::g_logger = mooon::sys::create_safe_logger();
            mooon::sys::CThreadEngine* signal_thread = new mooon::sys::CThreadEngine(mooon::sys::bind(&signal_thread_proc));

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

            signal_thread->join();
            delete signal_thread;
            return 0;
        }
        catch (mooon::sys::CSyscallException& ex)
        {
            MYLOG_ERROR("%s\n", ex.str().c_str());
            return 1;
        }
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

void move_thread_proc(uint8_t i)
{
    uint32_t num_moved = 0;
    const uint32_t interval = mooon::argument::interval->value();
    const std::string& src_key = get_src_key(i);
    const std::string& dst_key = get_dst_key(i);
    r3c::CRedisClient src_redis(mooon::argument::src_redis->value());
    r3c::CRedisClient dst_redis(mooon::argument::dst_redis->value());

    MYLOG_INFO("[%s] => [%s]\n", src_key.c_str(), dst_key.c_str());
    while (!g_stop)
    {
        std::string value;

        try
        {
            if (!src_redis.rpop(src_key, &value))
            {
                mooon::sys::CUtils::millisleep(interval);
            }
        }
        catch (r3c::CRedisException& ex)
        {
            MYLOG_ERROR("[%s]: %s\n", src_key.c_str(), ex.str().c_str());
            mooon::sys::CUtils::millisleep(interval);
            continue;
        }

        do
        {
            try
            {
                dst_redis.lpush(dst_key, value);
                if (0 == ++num_moved%mooon::argument::tick->value())
                {
                    MYLOG_INFO("[%s]=>[%s]: %u\n", src_key.c_str(), dst_key.c_str(), num_moved);
                }
                break;
            }
            catch (r3c::CRedisException& ex)
            {
                MYLOG_ERROR("[%s]=>[%s]: %s\n", src_key.c_str(), dst_key.c_str(), ex.str().c_str());
                mooon::sys::CUtils::millisleep(interval);
            }
        } while (!g_stop);
    }
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
