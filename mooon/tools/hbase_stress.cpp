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
#include "THBaseService.h"
#include <mooon/net/thrift_helper.h>
#include <mooon/sys/atomic.h>
#include <mooon/sys/safe_logger.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/args_parser.h>
#include <mooon/utils/string_utils.h>
#include <mooon/utils/tokener.h>

STRING_ARG_DEFINE(hbase, "192.168.0.2:9090,192.168.0.3:9090", "hbase thrift nodes");
STRING_ARG_DEFINE(table, "test", "table name");
STRING_ARG_DEFINE(prefix, "", "prefix of rowkey");
INTEGER_ARG_DEFINE(uint16_t, threads, 1, 1, 1000, "number of threads");
INTEGER_ARG_DEFINE(uint8_t, verbose, 0, 0, 1, "print error");
INTEGER_ARG_DEFINE(uint8_t, num_columns, 1, 1, std::numeric_limits<uint8_t>::max(), "number of columns");
INTEGER_ARG_DEFINE(uint32_t, num_rows, 1, 1, std::numeric_limits<uint32_t>::max(), "number of rows");
INTEGER_ARG_DEFINE(uint16_t, value_length, 10, 1, std::numeric_limits<uint16_t>::max(), "length of value");
INTEGER_ARG_DEFINE(uint16_t, timeout, 10, 1, std::numeric_limits<uint16_t>::max(), "timeout seconds of thrift");
INTEGER_ARG_DEFINE(uint8_t, test, 2, 0, 2, "0: test all, 1: test write only, 2: test read onley");

static atomic_t sg_success_count = 0;
static atomic_t sg_failure_count = 0;
static atomic_t sg_empty_count = 0; // 读操作未取到数据的个数
static atomic_t sg_thread_count = 0; // 当前仍然在工作的线程数

static void test_write(const std::map<std::string, std::string>& hbase_nodes);
static void test_read(const std::map<std::string, std::string>& hbase_nodes);
static void write_thread(uint16_t index, std::string hbase_ip, uint16_t hbase_port);
static void read_thread(uint16_t index, std::string hbase_ip, uint16_t hbase_port);

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }
    mooon::sys::g_logger = mooon::sys::create_safe_logger(true);

    mooon::utils::CEnhancedTokener tokener;
    tokener.parse(mooon::argument::hbase->value(), ",", ':');
    const std::map<std::string, std::string>& hbase_nodes = tokener.tokens();

    if (0 == mooon::argument::test->value())
    {
        test_write(hbase_nodes);
        test_read(hbase_nodes);
    }
    else if (1 == mooon::argument::test->value())
    {
        test_write(hbase_nodes);
    }
    else if (2 == mooon::argument::test->value())
    {
        test_read(hbase_nodes);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::string get_rowkey(uint32_t row, int index)
{
    const uint32_t random_number = mooon::sys::CUtils::get_random_number(row, std::numeric_limits<uint32_t>::max());
    if (mooon::argument::prefix->value().empty())
        return mooon::utils::CStringUtils::format_string("%u%u%d", random_number, row, index);
    else
        return mooon::utils::CStringUtils::format_string("%s_%u%u%d", mooon::argument::prefix->c_value(), random_number, row, index);
}

void test_write(const std::map<std::string, std::string>& hbase_nodes)
{
    try
    {
        mooon::sys::CStopWatch stop_watch;
        mooon::sys::CThreadEngine** stress_threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];

        atomic_set(&sg_thread_count, mooon::argument::threads->value());
        for (uint16_t i=0,j=0; i<mooon::argument::threads->value(); ++i,++j)
        {
            if (j >= hbase_nodes.size())
                j = 0;
            std::map<std::string, std::string>::const_iterator iter = hbase_nodes.begin();
            std::advance(iter, j);

            const std::string hbase_ip = iter->first;
            const uint16_t hbase_port = mooon::utils::CStringUtils::string2int<uint16_t>(iter->second);
            stress_threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(write_thread, i, hbase_ip, hbase_port));
        }

        for (uint16_t i=0; i<mooon::argument::threads->value(); ++i)
        {
            stress_threads[i]->join();
            delete stress_threads[i];
        }
        delete []stress_threads;

        uint32_t microseconds = stop_watch.get_elapsed_microseconds();
        uint32_t milliseconds = microseconds / 1000;
        uint32_t seconds = milliseconds / 1000;
        uint32_t success = static_cast<uint32_t>(atomic_read(&sg_success_count));
        uint32_t failure = static_cast<uint32_t>(atomic_read(&sg_failure_count));
        uint32_t qps = (0 == seconds)? (success+failure): (success+failure) / seconds;
        MYLOG_INFO("seconds: %u, milliseconds: %u\n", seconds, milliseconds);
        MYLOG_INFO("SUCCESS: %u, FAILURE: %u\n", success, failure);
        MYLOG_INFO("QPS: %u, %u\n", qps, qps/mooon::argument::threads->value());
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        MYLOG_ERROR("%s\n", ex.str().c_str());
    }
}

void test_read(const std::map<std::string, std::string>& hbase_nodes)
{
    try
    {
        mooon::sys::CStopWatch stop_watch;
        mooon::sys::CThreadEngine** stress_threads = new mooon::sys::CThreadEngine*[mooon::argument::threads->value()];

        atomic_set(&sg_thread_count, mooon::argument::threads->value());
        for (uint16_t i=0,j=0; i<mooon::argument::threads->value(); ++i,++j)
        {
            if (j >= hbase_nodes.size())
                j = 0;
            std::map<std::string, std::string>::const_iterator iter = hbase_nodes.begin();
            std::advance(iter, j);

            const std::string hbase_ip = iter->first;
            const uint16_t hbase_port = mooon::utils::CStringUtils::string2int<uint16_t>(iter->second);
            stress_threads[i] = new mooon::sys::CThreadEngine(mooon::sys::bind(read_thread, i, hbase_ip, hbase_port));
        }

        for (uint16_t i=0; i<mooon::argument::threads->value(); ++i)
        {
            stress_threads[i]->join();
            delete stress_threads[i];
        }
        delete []stress_threads;

        uint32_t microseconds = stop_watch.get_elapsed_microseconds();
        uint32_t milliseconds = microseconds / 1000;
        uint32_t seconds = milliseconds / 1000;
        uint32_t success = static_cast<uint32_t>(atomic_read(&sg_success_count));
        uint32_t failure = static_cast<uint32_t>(atomic_read(&sg_failure_count));
        uint32_t empty = static_cast<uint32_t>(atomic_read(&sg_empty_count));
        uint32_t qps = (0 == seconds)? (success+failure+empty): (success+failure+empty) / seconds;
        MYLOG_INFO("seconds: %u, milliseconds: %u\n", seconds, milliseconds);
        MYLOG_INFO("SUCCESS: %u, FAILURE: %u, EMPTY: %u\n", success, failure, empty);
        MYLOG_INFO("QPS: %u, %u\n", qps, qps/mooon::argument::threads->value());
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        MYLOG_ERROR("%s\n", ex.str().c_str());
    }
}

void write_thread(uint16_t index, std::string hbase_ip, uint16_t hbase_port)
{
    mooon::net::CThriftClientHelper<apache::hadoop::hbase::thrift2::THBaseServiceClient> hbase(hbase_ip, hbase_port, mooon::argument::timeout->value()*1000, mooon::argument::timeout->value()*1000, mooon::argument::timeout->value()*1000);

    for (uint32_t row=0; row<mooon::argument::num_rows->value(); ++row)
    {
        const std::string rowkey = get_rowkey(row, index);

        std::vector<apache::hadoop::hbase::thrift2::TColumnValue> columns_value(mooon::argument::num_columns->value());
        for (uint8_t col=0; col<mooon::argument::num_columns->value(); ++col)
        {
            const std::string column_name = mooon::utils::CStringUtils::format_string("field%d", col);
            const std::string column_value(mooon::argument::value_length->value(), '#');
            columns_value[col].__set_family("cf1");
            columns_value[col].__set_qualifier(column_name);
            columns_value[col].__set_value(column_value);
        }

        apache::hadoop::hbase::thrift2::TPut put;
        put.__set_row(rowkey);
        put.__set_columnValues(columns_value);

        bool to_continue = true;
        while (true)
        {
            const time_t begin = time(NULL);

            try
            {
                if (!hbase.is_connected())
                    hbase.connect();

                hbase->put(mooon::argument::table->value(), put);
                //const time_t end = time(NULL);
                //atomic_inc(&sg_success_count);
				const uint32_t success_count = static_cast<uint32_t>(atomic_add_return(1, &sg_success_count));
				if (0 == success_count%10000)
				{
					MYLOG_INFO("number of write: %u\n", success_count);
				}

                break;
            }
            catch (apache::thrift::transport::TTransportException& ex)
            {
                const time_t end = time(NULL);
                MYLOG_ERROR("(%u) TransportException(%s): %s\n", static_cast<unsigned>(end-begin), hbase.str().c_str(), ex.what());
                hbase.close();
                atomic_inc(&sg_failure_count);
            }
            catch (apache::thrift::TApplicationException& ex)
            {
                MYLOG_ERROR("ApplicationException(%s): %s\n", hbase.str().c_str(), ex.what());
                atomic_inc(&sg_failure_count);
                to_continue = false;
                break;
            }
            catch (apache::thrift::TException& ex)
            {
                MYLOG_ERROR("ThriftException(%s): %s\n", hbase.str().c_str(), ex.what());
                atomic_inc(&sg_failure_count);
                to_continue = false;
                break;
            }
        }
        if (!to_continue)
            break;
    }

    atomic_dec(&sg_thread_count);
    MYLOG_INFO("write thread[%u] ended: %d\n", static_cast<unsigned int>(pthread_self()), static_cast<int>(atomic_read(&sg_thread_count)));
}

void read_thread(uint16_t index, std::string hbase_ip, uint16_t hbase_port)
{
    mooon::net::CThriftClientHelper<apache::hadoop::hbase::thrift2::THBaseServiceClient> hbase(hbase_ip, hbase_port, mooon::argument::timeout->value()*1000, mooon::argument::timeout->value()*1000, mooon::argument::timeout->value()*1000);

    uint32_t print_number = 0;
    for (uint32_t row=0; row<mooon::argument::num_rows->value(); ++row)
    {
        const std::string rowkey = get_rowkey(row, index);
        apache::hadoop::hbase::thrift2::TGet input;
        input.__set_row(rowkey);

        std::vector<apache::hadoop::hbase::thrift2::TColumn> columns(mooon::argument::num_columns->value());
        for (uint8_t col=0; col<mooon::argument::num_columns->value(); ++col)
        {
            const std::string column_name = mooon::utils::CStringUtils::format_string("field%d", col);
            columns[col].__set_family("cf1");
            columns[col].__set_qualifier(column_name);
        }
        input.__set_columns(columns);

        bool to_continue = true;
        while (true)
        {
            try
            {
                if (!hbase.is_connected())
                    hbase.connect();

                apache::hadoop::hbase::thrift2::TResult result;
                hbase->get(result, mooon::argument::table->value(), input);

                const std::vector<apache::hadoop::hbase::thrift2::TColumnValue>& columns_value = result.columnValues;
                if (columns_value.empty())
                {
                    atomic_inc(&sg_empty_count);
                }
                else
                {
                    for (std::vector<apache::hadoop::hbase::thrift2::TColumnValue>::size_type col=0; col<columns_value.size(); ++col)
                    {
                        const apache::hadoop::hbase::thrift2::TColumnValue& column_value = columns_value[col];

                        if (print_number < 10)
                        {
                            MYLOG_INFO("[%s][%zd] %s\n", result.row.c_str(), col, column_value.value.c_str());
                        }
                    }

                    ++print_number;
                    atomic_inc(&sg_success_count);
                }

                break;
            }
            catch (apache::thrift::transport::TTransportException& ex)
            {
                MYLOG_ERROR("TransportException(%s): %s\n", hbase.str().c_str(), ex.what());
                hbase.close();
                atomic_inc(&sg_failure_count);
            }
            catch (apache::thrift::TApplicationException& ex)
            {
                MYLOG_ERROR("ApplicationException(%s): %s\n", hbase.str().c_str(), ex.what());
                atomic_inc(&sg_failure_count);
                to_continue = false;
                break;
            }
            catch (apache::thrift::TException& ex)
            {
                MYLOG_ERROR("ThriftException(%s): %s\n", hbase.str().c_str(), ex.what());
                atomic_inc(&sg_failure_count);
                to_continue = false;
                break;
            }
        }
        if (!to_continue)
            break;
    }

    atomic_dec(&sg_thread_count);
    MYLOG_INFO("read thread[%u] ended: %d\n", static_cast<unsigned int>(pthread_self()), static_cast<int>(atomic_read(&sg_thread_count)));
}

