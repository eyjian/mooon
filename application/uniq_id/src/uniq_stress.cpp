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

// UniqId压力测试工具

static void usage();
static void thread_proc(uint64_t times, const char* agent_nodes, bool polling);

// Usage1: uniq_stress agent_nodes
// Usage2: uniq_stress agent_nodes times
// Usage3: uniq_stress agent_nodes times concurrency
// Usage4: uniq_stress agent_nodes times concurrency poll
int main(int argc, char* argv[])
{
	if ((argc != 2) && (argc != 3) && (argc != 4) && (argc != 5))
	{
		usage();
		exit(1);
	}

	bool polling = false;
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
    if (argc >= 5)
    {
        polling = true;
    }

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
            thread_engine = new mooon::sys::CThreadEngine(mooon::sys::bind(&thread_proc, times, agent_nodes, polling));
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
	fprintf(stderr, "Usage1: uniq_stress agent_nodes\n");
	fprintf(stderr, "Usage2: uniq_stress agent_nodes times\n");
	fprintf(stderr, "Usage3: uniq_stress agent_nodes times concurrency\n");
	fprintf(stderr, "Usage4: uniq_stress agent_nodes times concurrency poll\n");
}

void thread_proc(uint64_t times, const char* agent_nodes, bool polling)
{
    uint32_t timeout_milliseconds = 200;
    uint8_t retry_times = 5;

    for (uint64_t i=0; i<times; ++i)
    {
        try
        {
            mooon::CUniqId uniq_id(agent_nodes, timeout_milliseconds, retry_times, polling);
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
