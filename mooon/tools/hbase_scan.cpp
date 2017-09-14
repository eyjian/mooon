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

STRING_ARG_DEFINE(hbase_ip, "127.0.0.1", "hbase thrift server IP");
INTEGER_ARG_DEFINE(uint16_t, hbase_port, 9090, 1000, 65535, "hbase thrift server port");
STRING_ARG_DEFINE(table, "test", "hbase table name");
STRING_ARG_DEFINE(family, "cf1", "hbase column family name");
STRING_ARG_DEFINE(startrow, "", "hbase row key");
STRING_ARG_DEFINE(stoprow, "", "hbase row key");
INTEGER_ARG_DEFINE(int, num, 1, 1, 100000, "number of rows to scan");
INTEGER_ARG_DEFINE(int, timeout, 10, 1, 3600, "timeout seconds of thrift");

using namespace apache::hadoop;

int main(int argc, char* argv[])
{
    std::string errmsg;
    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }

    if (mooon::argument::hbase_ip->value().empty())
    {
        fprintf(stderr, "parameter[--hbase_ip] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    if (mooon::argument::table->value().empty())
    {
        fprintf(stderr, "parameter[--table] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    if (mooon::argument::family->value().empty())
    {
        fprintf(stderr, "parameter[--family] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }
    if (mooon::argument::startrow->value().empty())
    {
        fprintf(stderr, "parameter[--startrow] not set\n");
        fprintf(stderr, "%s\n", mooon::utils::g_help_string.c_str());
        exit(1);
    }

    try
    {
        mooon::sys::g_logger = mooon::sys::create_safe_logger("/tmp", "hbase_scan.log");
        mooon::sys::g_logger->set_backup_number(2);
        mooon::sys::g_logger->set_single_filesize(1024*1024);

        mooon::net::CThriftClientHelper<apache::hadoop::hbase::thrift2::THBaseServiceClient> hbase(mooon::argument::hbase_ip->value(), mooon::argument::hbase_port->value(), mooon::argument::timeout->value()*1000, mooon::argument::timeout->value()*1000, mooon::argument::timeout->value()*1000);
        std::vector<hbase::thrift2::TResult> results;
        hbase::thrift2::TScan scan;

        hbase.connect();
        MYLOG_INFO("connect hbase ok\n");

        scan.startRow = mooon::argument::startrow->value();
        if (!mooon::argument::stoprow->value().empty())
            scan.stopRow = mooon::argument::stoprow->value();

        hbase->getScannerResults(results, mooon::argument::table->value(), scan, mooon::argument::num->value());
        for (std::vector<hbase::thrift2::TResult>::size_type row=0; row<results.size(); ++row)
        {
            const hbase::thrift2::TResult& result = results[row];
            MYLOG_INFO("ROWKEY[%s] =>\n", result.row.c_str());
            printf("ROWKEY[%s] =>\n", result.row.c_str());

            for (std::vector<hbase::thrift2::TColumnValue>::size_type col=0; col<result.columnValues.size(); ++col)
            {
                const hbase::thrift2::TColumnValue& column = result.columnValues[col];
                MYLOG_INFO("\tfamily => %s\n", column.family.c_str());
                MYLOG_INFO("\t\tqualifier => %s\n", column.qualifier.c_str());
                MYLOG_INFO("\t\t\tvalue => %s\n", column.value.c_str());
                MYLOG_INFO("\t\t\t\ttimestamp => %" PRId64"\n", column.timestamp);

                printf("\tfamily => %s\n", column.family.c_str());
                printf("\t\tqualifier => %s\n", column.qualifier.c_str());
                printf("\t\t\tvalue => %s\n", column.value.c_str());
                printf("\t\t\t\ttimestamp => %" PRId64"\n", column.timestamp);
            }
        }
    }
    catch (hbase::thrift2::TIOError& ex)
    {
        MYLOG_ERROR("%s\n", ex.message.c_str());
    }
    catch (apache::thrift::transport::TTransportException& ex)
    {
        MYLOG_ERROR("thrift transport exception: (%d)%s\n", ex.getType(), ex.what());
    }
    catch (apache::thrift::TApplicationException& ex)
    {
        MYLOG_ERROR("%s\n", ex.what());
    }
    catch (apache::thrift::TException& ex)
    {
        MYLOG_ERROR("%s\n", ex.what());
    }

    return 0;
}
