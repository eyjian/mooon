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

static void usage();
static void thread_proc(uint64_t times, const char* agent_nodes);

// 生成带日期的交易流水号示例
static void print_transaction_id(const char* agent_nodes);

// Usage1: uniq_cli agent_nodes
// Usage2: uniq_cli agent_nodes times
// Usage3: uniq_cli agent_nodes times concurrency
int main(int argc, char* argv[])
{
	if ((argc != 2) && (argc != 3) && (argc != 4))
	{
		usage();
		exit(1);
	}

	uint64_t times = 1;
	uint64_t concurrency = 1;
	const char* agent_nodes = argv[1];
	if (argc >= 3)
	{
	    if (!mooon::utils::CStringUtils::string2int(argv[2], times))
	        times = 1;
	}
	if (argc >= 4)
	{
        if (!mooon::utils::CStringUtils::string2int(argv[3], concurrency))
            concurrency = 1;
	}

    print_transaction_id(agent_nodes);
    fprintf(stdout, "agent_nodes: %s\n", agent_nodes);
    fprintf(stdout, "times: %"PRIu64", concurrency: %"PRIu64"\n", times, concurrency);

    try
    {
        uint64_t i = 0;
        mooon::sys::CThreadEngine* thread_engine;
        std::vector<mooon::sys::CThreadEngine*> thread_pool(concurrency);
        mooon::sys::CStopWatch stop_watch;
        for (i=0; i<concurrency; ++i)
        {
            thread_engine = new mooon::sys::CThreadEngine(mooon::sys::bind(&thread_proc, times, agent_nodes));
            thread_pool[i] = thread_engine;
        }
        for (i=0; i<concurrency; ++i)
        {
            thread_engine = thread_pool[i];
            thread_engine->join();
            delete thread_engine;
        }
        thread_pool.clear();

        unsigned int total_microseconds = stop_watch.get_total_elapsed_microseconds();
        fprintf(stdout, "%.2fms, %0.2fms, %.2f/s\n", (double)total_microseconds/1000, (double)total_microseconds/(1000*times*concurrency), (double)(times*concurrency*1000000)/total_microseconds);
    }
    catch (mooon::sys::CSyscallException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }

	return 0;
}

void usage()
{
	fprintf(stderr, "Usage1: uniq_cli agent_nodes\n");
	fprintf(stderr, "Usage2: uniq_cli agent_nodes times\n");
	fprintf(stderr, "Usage3: uniq_cli agent_nodes times concurrency\n");
}

void thread_proc(uint64_t times, const char* agent_nodes)
{
    for (uint64_t i=0; i<times; ++i)
    {
        try
        {
            mooon::CUniqId uniq_id(agent_nodes);
#if 1
            uint64_t uid = uniq_id.get_uniq_id();
#else
            uint64_t uid = uniq_id.get_local_uniq_id();
#endif
            union mooon::UniqID uid_struct;
            uid_struct.value = uid;

            if ((0 == i) || (0 == i%100000) || (i == times-1))
                fprintf(stdout, "[%"PRIu64"]uid: %"PRIu64" => %s\n", mooon::sys::CThreadEngine::get_current_thread_id(), uid, uid_struct.id.str().c_str());
        }
        catch (mooon::sys::CSyscallException& ex)
        {
            fprintf(stderr, "%s\n", ex.str().c_str());
            break;
        }
        catch (mooon::utils::CException& ex)
        {
            fprintf(stderr, "%s\n", ex.str().c_str());
            break;
        }
    }
}

void print_transaction_id(const char* agent_nodes)
{
    try
    {
        time_t now = time(NULL);
        struct tm* tm = localtime(&now);
        mooon::CUniqId uniq_id(agent_nodes);

        std::string str;

        str = uniq_id.get_transaction_id("02%L%Y%M%D%m%S");
        fprintf(stdout, "[A]NO.: %s\n", str.c_str());
        str = uniq_id.get_transaction_id("%3d%L%Y%M%D%H%S", 9);
        fprintf(stdout, "[B]NO.: %s\n", str.c_str());
        str = uniq_id.get_transaction_id("%2d%L%Y%M%D%H%m%S%s", 9, "XX");
        fprintf(stdout, "[C]NO.: %s\n", str.c_str());

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
