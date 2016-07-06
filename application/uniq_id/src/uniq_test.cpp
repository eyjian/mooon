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
#include <mooon/uniq_id/uniq_id.h>
#include <mooon/sys/stop_watch.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/utils/string_utils.h>
#include <time.h>
#include <vector>

// UniqId测试程序

static void usage();

// 生成带日期的交易流水号示例
// polling 控制取agent的方式，为true表示轮询，否则表示随机
static void print_transaction_id(const char* agent_nodes, bool polling);

// Usage1: uniq_cli agent_nodes
// Usage2: uniq_cli agent_nodes poll
int main(int argc, char* argv[])
{
	if ((argc != 2) && (argc != 3))
	{
		usage();
		exit(1);
	}

	bool polling = false;
	const char* agent_nodes = argv[1];
    if (argc > 2)
    {
        polling = true;
    }

    print_transaction_id(agent_nodes, polling);
    fprintf(stdout, "agent_nodes: %s\n", agent_nodes);

	return 0;
}

void usage()
{
	fprintf(stderr, "Usage1: uniq_cli agent_nodes\n");
	fprintf(stderr, "Usage2: uniq_cli agent_nodes poll\n");
}

void print_transaction_id(const char* agent_nodes, bool polling)
{
    try
    {
        uint16_t num = 3;
        std::string str;

        time_t now = time(NULL);
        struct tm* tm = localtime(&now);
        uint32_t timeout_milliseconds = 200;
        uint8_t retry_times = 5;
        mooon::CUniqId uniq_id(agent_nodes, timeout_milliseconds, retry_times, polling);

        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%5S");
        fprintf(stdout, "[A1]NO.: %s\n", str.c_str());
        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%7S");
        fprintf(stdout, "[A2]NO.: %s\n", str.c_str());
        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%9S");
        fprintf(stdout, "[A3]NO.: %s\n\n", str.c_str());

        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%8S%s", "##");
        fprintf(stdout, "[B1]NO.: %s\n", str.c_str());
        str = uniq_id.get_transaction_id("%s02%L%Y%M%D%m%8S", "##");
        fprintf(stdout, "[B2]NO.: %s\n\n", str.c_str());

        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%8S%2X", 31);
        fprintf(stdout, "[C1]NO.: %s\n", str.c_str());
        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%8S%2X", 1000);
        fprintf(stdout, "[C2]NO.: %s\n\n", str.c_str());

        // 批量取流水号
        std::vector<std::string> str_id_vec;
        uniq_id.get_transaction_id(num, &str_id_vec, "%3d%L%Y%M%D%H%S", 9);
        for (uint16_t i=0; i<num; ++i)
            fprintf(stdout, "[%d] %s\n", i, str_id_vec[i].c_str());
        fprintf(stdout, "\n");

        // 批量取4字节Seq
        uint32_t seq = uniq_id.get_unqi_seq(3);
        for (uint16_t i=0; i<num; ++i, ++seq)
            fprintf(stdout, "seq: %u\n", seq);
        seq = uniq_id.get_unqi_seq();
        fprintf(stdout, "seq: %u\n", seq);
        fprintf(stdout, "\n");

        // 批量取8字节ID
        std::vector<uint64_t> int_id_vec;
        uniq_id.get_local_uniq_id(num, &int_id_vec);
        for (uint16_t i=0; i<num; ++i)
        {
            uint64_t id64 = int_id_vec[i];
            union mooon::UniqID uid;
            uid.value = id64;
            fprintf(stdout, "id: %"PRIu64" => %s\n", id64, uid.id.str().c_str());
        }
        fprintf(stdout, "\n");

        for (int i=0; i<2; ++i)
        {
            uint8_t label = 0;
            uint32_t seq = 0; // 12345678
            uniq_id.get_label_and_seq(&label, &seq);

            // label转成十六进制字符，
            // 如果30位的seq（10亿，最大值为1073741823）在一天内消耗不完，则时间取到天即可，
            // 如果30位的seq在一天内会被消耗完，则时间应当取到小时，
            // 如果30位的seq在一小时内会被消耗完，则时间应当取到分钟。。。
            fprintf(stdout, "[%d]NO.: %02X%04d%02d%02d%02d%09u\n", i, (int)label, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, seq);
        }
        fprintf(stdout, "\n");
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }
    catch (mooon::utils::CException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
    }
}
