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
// 基于r3c实现的redis性能测试工具（r3c是一个在hiredis上实现的redis cluster c++客户端库）
//
// 运行示例：
// r3c_stress --redis=192.168.0.88:6379 --requests=100000 --threads=20
#include <r3c/r3c.h>
#include <mooon/sys/atomic.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>

// 运行模式，如果是清理，需要保持参数和测试时相同
INTEGER_ARG_DEFINE(uint8_t, test, 1, 0, 1, "1: test, 0: clean test data");

STRING_ARG_DEFINE(redis, "127.0.0.1:6379", "redis nodes");
INTEGER_ARG_DEFINE(uint8_t, threads, 1, 1, 100, "number of threads");
INTEGER_ARG_DEFINE(uint32_t, requests, 1, 1, std::numeric_limits<int32_t>::max(), "number of requests of every threads");
STRING_ARG_DEFINE(prefix, "r3ct", "key prefix"); // 前缀的一个作用是方便清掉测试时产生的数据
INTEGER_ARG_DEFINE(uint32_t, expire, 60, 1, 3600, "key expired seconds");
INTEGER_ARG_DEFINE(uint8_t, verbose, 0, 0, 1, "print error");
INTEGER_ARG_DEFINE(uint8_t, increments, 10, 1, 100, "number of increments for hmincrby");
INTEGER_ARG_DEFINE(uint16_t, value_length, 10, 1, std::numeric_limits<uint16_t>::max(), "length of value");

static atomic_t sg_success = 0;
static atomic_t sg_failure = 0;
static atomic_t sg_not_exists = 0;

static bool is_clean_mode() { return 0 == mooon::argument::test->value(); }
static bool is_test_mode() { return 1 == mooon::argument::test->value(); }

static void set_test();
static void get_test();
static void setnx_test();
static void setex_test();
static void setnxex_test();
static void hset_test();
static void hget_test();
static void hmincrby_test();
static void lpush_test();
static void rpop_test();

static void set_stress_thread(uint8_t index);
static void get_stress_thread(uint8_t index);
static void setnx_stress_thread(uint8_t index);
static void setex_stress_thread(uint8_t index);
static void setnxex_stress_thread(uint8_t index);
static void hset_stress_thread(uint8_t index);
static void hget_stress_thread(uint8_t index);
static void hmincrby_stress_thread(uint8_t index);
static void lpush_stress_thread(uint8_t index);
static void rpop_stress_thread(uint8_t index);

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        if (!errmsg.empty())
            fprintf(stderr, "%s\n", errmsg.c_str());
        else
            fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }

    try
    {
        // KV
        set_test();
        if (is_test_mode())
            get_test();
        setnx_test();
        setex_test();
        setnxex_test();

        // HASH
        hset_test();
        if (is_test_mode())
            hget_test();
        hmincrby_test();

        // QUEUE
        lpush_test();
        if (is_test_mode())
            rpop_test();

        return 0;
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }
}

////////////////////////////////////////////////////////////////////////////////
void set_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

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

    if (is_test_mode())
    {
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

void setnx_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(setnx_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;

    if (is_test_mode())
    {
        unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
        unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
        unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
        unsigned int success = atomic_read(&sg_success);
        unsigned int failure = atomic_read(&sg_failure);
        unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
        fprintf(stdout, "\nsetnx:\n");
        fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
        fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
        fprintf(stdout, "qps: %u\n", qps);
    }
}

void setex_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(setex_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;

    if (is_test_mode())
    {
        unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
        unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
        unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
        unsigned int success = atomic_read(&sg_success);
        unsigned int failure = atomic_read(&sg_failure);
        unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
        fprintf(stdout, "\nsetex:\n");
        fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
        fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
        fprintf(stdout, "qps: %u\n", qps);
    }
}

void setnxex_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(setnxex_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;

    if (is_test_mode())
    {
        unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
        unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
        unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
        unsigned int success = atomic_read(&sg_success);
        unsigned int failure = atomic_read(&sg_failure);
        unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
        fprintf(stdout, "\nsetnxex:\n");
        fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
        fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
        fprintf(stdout, "qps: %u\n", qps);
    }
}

////////////////////////////////////////////////////////////////////////////////
void hset_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(hset_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;

    if (is_test_mode())
    {
        unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
        unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
        unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
        unsigned int success = atomic_read(&sg_success);
        unsigned int failure = atomic_read(&sg_failure);
        unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
        fprintf(stdout, "\nhset:\n");
        fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
        fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
        fprintf(stdout, "qps: %u\n", qps);
    }
}

void hget_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(hget_stress_thread, i));
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
    fprintf(stdout, "\nhget:\n");
    fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
    fprintf(stdout, "total: %u, success: %u, failure: %u, not exists: %u\n", success+failure, success, failure, not_exists);
    fprintf(stdout, "qps: %u\n", qps);
}

void hmincrby_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(hmincrby_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;

    if (is_test_mode())
    {
        unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
        unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
        unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
        unsigned int success = atomic_read(&sg_success);
        unsigned int failure = atomic_read(&sg_failure);
        unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
        fprintf(stdout, "\nhmincrby:\n");
        fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
        fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
        fprintf(stdout, "qps: %u\n", qps);
    }
}

////////////////////////////////////////////////////////////////////////////////
void lpush_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(lpush_stress_thread, i));
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
    {
        threads[i]->join();
        delete threads[i];
    }
    delete []threads;

    if (is_test_mode())
    {
        unsigned int elapsed_microseconds = stop_watch.get_elapsed_microseconds();
        unsigned int elapsed_milliseconds = elapsed_microseconds / 1000;
        unsigned int elapsed_seconds = elapsed_milliseconds / 1000;
        unsigned int success = atomic_read(&sg_success);
        unsigned int failure = atomic_read(&sg_failure);
        unsigned int qps = (0 == elapsed_seconds)? (success+failure): (success+failure)/elapsed_seconds;
        fprintf(stdout, "\nlpush:\n");
        fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
        fprintf(stdout, "total: %u, success: %u, failure: %u\n", success+failure, success, failure);
        fprintf(stdout, "qps: %u\n", qps);
    }
}

void rpop_test()
{
    atomic_set(&sg_success, 0);
    atomic_set(&sg_failure, 0);
    atomic_set(&sg_not_exists, 0);

    mooon::sys::CStopWatch stop_watch;
    mooon::sys::CThreadEngine** threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];
    for (uint8_t i=0; i<mooon::argument::threads->value(); ++i)
        threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(rpop_stress_thread, i));
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
    fprintf(stdout, "\nrpop:\n");
    fprintf(stdout, "microseconds=%u, milliseconds=%u, seconds=%u\n", elapsed_microseconds, elapsed_milliseconds, elapsed_seconds);
    fprintf(stdout, "total: %u, success: %u, failure: %u, not exists: %u\n", success+failure, success, failure, not_exists);
    fprintf(stdout, "qps: %u\n", qps);
}

////////////////////////////////////////////////////////////////////////////////
void set_stress_thread(uint8_t index)
{
    try
    {
        const std::string value(mooon::argument::value_length->value(), '*');
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            const std::string& key = mooon::utils::CStringUtils::format_string("%s_%d_%u", mooon::argument::prefix->c_value(), index, i);

            try
            {
                if (is_clean_mode())
                {
                    redis.del(key);
                }
                else
                {
                    redis.set(key, value);
                    atomic_inc(&sg_success);
                }
            }
            catch (r3c::CRedisException& ex)
            {
                atomic_inc(&sg_failure);
                if (1 == mooon::argument::verbose->value())
                    fprintf(stderr, "SET [%s] ERROR: %s\n", key.c_str(), ex.str().c_str());
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void get_stress_thread(uint8_t index)
{
    try
    {
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            const std::string& key = mooon::utils::CStringUtils::format_string("%s_%d_%u", mooon::argument::prefix->c_value(), index, i);

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
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void setnx_stress_thread(uint8_t index)
{
    try
    {
        const std::string value(mooon::argument::value_length->value(), '*');
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            const std::string& key = mooon::utils::CStringUtils::format_string("%s_nx_%d_%u", mooon::argument::prefix->c_value(), index, i);

            try
            {
                if (is_clean_mode())
                {
                    redis.del(key);
                }
                else
                {
                    redis.setnx(key, value);
                    atomic_inc(&sg_success);
                }
            }
            catch (r3c::CRedisException& ex)
            {
                atomic_inc(&sg_failure);
                if (1 == mooon::argument::verbose->value())
                    fprintf(stderr, "SET [%s] ERROR: %s\n", key.c_str(), ex.str().c_str());
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void setex_stress_thread(uint8_t index)
{
    try
    {
        const std::string value(mooon::argument::value_length->value(), '*');
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            const std::string& key = mooon::utils::CStringUtils::format_string("%s_ex_%d_%u", mooon::argument::prefix->c_value(), index, i);

            try
            {
                if (is_clean_mode())
                {
                    redis.del(key);
                }
                else
                {
                    const uint32_t expired_seconds = mooon::argument::expire->value();
                    redis.setex(key, value, expired_seconds);
                    atomic_inc(&sg_success);
                }
            }
            catch (r3c::CRedisException& ex)
            {
                atomic_inc(&sg_failure);
                if (1 == mooon::argument::verbose->value())
                    fprintf(stderr, "SET [%s] ERROR: %s\n", key.c_str(), ex.str().c_str());
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void setnxex_stress_thread(uint8_t index)
{
    try
    {
        const std::string value(mooon::argument::value_length->value(), '*');
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            const std::string& key = mooon::utils::CStringUtils::format_string("%s_nxex_%d_%u", mooon::argument::prefix->c_value(), index, i);

            try
            {
                if (is_clean_mode())
                {
                    redis.del(key);
                }
                else
                {
                    const uint32_t expired_seconds = mooon::argument::expire->value();
                    redis.setnxex(key, value, expired_seconds);
                    atomic_inc(&sg_success);
                }
            }
            catch (r3c::CRedisException& ex)
            {
                atomic_inc(&sg_failure);
                if (1 == mooon::argument::verbose->value())
                    fprintf(stderr, "SET [%s] ERROR: %s\n", key.c_str(), ex.str().c_str());
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////
void hset_stress_thread(uint8_t index)
{
    try
    {
        const std::string value(mooon::argument::value_length->value(), '*');
        const std::string& key = mooon::utils::CStringUtils::format_string("%s_hash_%d", mooon::argument::prefix->c_value(), index);
        r3c::CRedisClient redis(mooon::argument::redis->value());

        if (is_clean_mode())
        {
            redis.del(key);
        }
        else
        {
            for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
            {
                const std::string field = mooon::utils::CStringUtils::format_string("field_%u", i);

                try
                {
                    redis.hset(key, field, value);
                    atomic_inc(&sg_success);
                }
                catch (r3c::CRedisException& ex)
                {
                    atomic_inc(&sg_failure);
                    if (1 == mooon::argument::verbose->value())
                        fprintf(stderr, "HSET [%s:%s] ERROR: %s\n", key.c_str(), field.c_str(), ex.str().c_str());
                }
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void hget_stress_thread(uint8_t index)
{
    try
    {
        const std::string& key = mooon::utils::CStringUtils::format_string("%s_hash_%d", mooon::argument::prefix->c_value(), index);
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            const std::string& field = mooon::utils::CStringUtils::format_string("field_%u", i);

            try
            {
                std::string value;

                if (redis.hget(key, field, &value))
                {
                    atomic_inc(&sg_success);
                }
                else
                {
                    atomic_inc(&sg_not_exists);
                    if (1 == mooon::argument::verbose->value())
                        fprintf(stderr, "HASH[%s:%s] not exists\n", key.c_str(), field.c_str());
                }
            }
            catch (r3c::CRedisException& ex)
            {
                atomic_inc(&sg_failure);
                if (1 == mooon::argument::verbose->value())
                    fprintf(stderr, "HGET [%s:%s] ERROR: %s\n", key.c_str(), field.c_str(), ex.str().c_str());
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void hmincrby_stress_thread(uint8_t index)
{
    try
    {
        const std::string& key = mooon::utils::CStringUtils::format_string("%s_eval_%d", mooon::argument::prefix->c_value(), index);
        r3c::CRedisClient redis(mooon::argument::redis->value());

        if (is_clean_mode())
        {
            redis.del(key);
        }
        else
        {
            for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
            {
                std::vector<std::pair<std::string, int64_t> > increments(mooon::argument::increments->value());
                for (std::vector<std::pair<std::string, int64_t> >::size_type i=0; i<increments.size(); ++i)
                {
                    increments[i].first = mooon::utils::CStringUtils::int_tostring(i%100);
                    increments[i].second = i;
                }

                try
                {
                    redis.hmincrby(key, increments);
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
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////
void lpush_stress_thread(uint8_t index)
{
    try
    {
        const std::string value(mooon::argument::value_length->value(), '*');
        const std::string& key = mooon::utils::CStringUtils::format_string("%s_queue_%d", mooon::argument::prefix->c_value(), index);
        r3c::CRedisClient redis(mooon::argument::redis->value());

        if (is_clean_mode())
        {
            redis.del(key);
        }
        else
        {
            for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
            {
                try
                {
                    redis.lpush(key, value);
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
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}

void rpop_stress_thread(uint8_t index)
{
    try
    {
        const std::string& key = mooon::utils::CStringUtils::format_string("%s_queue_%d", mooon::argument::prefix->c_value(), index);
        r3c::CRedisClient redis(mooon::argument::redis->value());

        for (uint32_t i=0; i<mooon::argument::requests->value(); ++i)
        {
            try
            {
                std::string value;

                if (redis.rpop(key, &value))
                {
                    atomic_inc(&sg_success);
                }
                else
                {
                    break;
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
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "[%s] %s\n", __FUNCTION__, ex.str().c_str());
    }
}
