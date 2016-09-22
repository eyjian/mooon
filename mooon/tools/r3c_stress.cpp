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
#include <r3c/r3c.h>
#include <mooon/sys/atomic.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>

STRING_ARG_DEFINE(redis, "127.0.0.1:6379", "redis nodes");
INTEGER_ARG_DEFINE(uint8_t, threads, 1, 1, 100, "number of threads");
INTEGER_ARG_DEFINE(uint32_t, requests, 1, 1, std::numeric_limits<int32_t>::max(), "number of requests of every threads");
STRING_ARG_DEFINE(prefix, "r3c", "key prefix");
INTEGER_ARG_DEFINE(uint32_t, expire, 60, 1, 3600, "key expired seconds");
INTEGER_ARG_DEFINE(uint8_t, verbose, 0, 0, 1, "print error");

static atomic_t sg_success = 0;
static atomic_t sg_failure = 0;
static atomic_t sg_not_exists = 0;

static void set_test();
static void get_test();
static void set_stress_thread(uint8_t index);
static void get_stress_thread(uint8_t index);

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }

    r3c::set_debug_log_write(NULL);
    r3c::set_info_log_write(NULL);
    r3c::set_error_log_write(NULL);
    set_test();
    get_test();

    return 0;
}

void set_test()
{
    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(set_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;
    unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
    unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
    unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
    unsigned int success = atomic_read(&sg_success);
    unsigned int failure = atomic_read(&sg_failure);
    unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
    fprintf(stdout, "set:\n");
    fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
    fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
    fprintf(stdout, "qps: %u\n", qps);
}

void get_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(get_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;
    unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
    unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
    unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
    unsigned int success = atomic_read(&sg_success);
    unsigned int failure = atomic_read(&sg_failure);
    unsigned int not_exists = atomic_read(&sg_not_exists);
    unsigned int qps = (0 == elapsed_seconds)? (success+failure+not_exists): (success+failure+not_exists)/elapsed_seconds;
    fprintf(stdout, "\nget:\n");
    fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
    fprintf(stdout, "total: %u, success: %u, failure: %u, not exists: %u\n", success+failure, success, failure, not_exists);
    fprintf(stdout, "qps: %u\n", qps);
}

void set_stress_thread(uint8_t index)
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    r3c::CRedisClient redis(mooon::argument::redis->value());
    for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
    {
        const std::string key = mooon::utils::CStringUtils::format_string("%s_%d_%u", mooon::argument::prefix->c_value(), index, i);

        try
        {
            const std::string value = mooon::utils::CStringUtils::int_tostring(i);
            const uint32_t expired_seconds = mooon::argument::expire->value();
            redis.setex(key, value, expired_seconds);
            atomic_inc(&sg_success);
        }
        catch (r3c::CRedisException& ex)
        {
            atomic_inc(&sg_failure);
            if (1 == mooon::argument::verbose->value())
                fprintf(stderr, "SET [%s] ERROR: %s\n", key.c_str(), ex.str().c_str());
        }
    }
}

void get_stress_thread(uint8_t index)
{
    r3c::CRedisClient redis(mooon::argument::redis->value());
    for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
    {
        const std::string key = mooon::utils::CStringUtils::format_string("%s_%d_%u", mooon::argument::prefix->c_value(), index, i);

        try
        {
            std::string value;
            if (redis.get(key, &value))
            {
                atomic_inc(&sg_success);
            }
            else
            {
                atomic_inc(&sg_not_exists);
                if (1 == mooon::argument::verbose->value())
                    fprintf(stderr, "[%s] not exists\n", key.c_str());
            }
        }
        catch (r3c::CRedisException& ex)
        {
            atomic_inc(&sg_failure);
            if (1 == mooon::argument::verbose->value())
                fprintf(stderr, "GET [%s] ERROR: %s\n", key.c_str(), ex.str().c_str());
        }
    }
}
