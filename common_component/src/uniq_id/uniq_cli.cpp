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
#include <vector>

static void usage();
static void thread_proc(int times, const char* agent_nodes);

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

	int times = 1;
	int concurrency = 1;
	const char* agent_nodes = argv[1];
	if (argc >= 3)
		times = atoi(argv[2]);
	if (argc >= 4)
	    concurrency = atoi(argv[3]);
	if (times < 1)
	    times = 1;
    if (concurrency < 1)
        concurrency = 1;
    fprintf(stdout, "agent_nodes: %s\n", agent_nodes);
    fprintf(stdout, "times: %d, concurrency: %d\n", times, concurrency);

    try
    {
        int i = 0;
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

        unsigned int microseconds = stop_watch.get_total_elapsed_microseconds();
        fprintf(stdout, "%.2fus, %0.2fs\n", (double)microseconds/1000, (double)microseconds/(1000*times*concurrency));
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

void thread_proc(int times, const char* agent_nodes)
{
    for (int i=0; i<times; ++i)
    {
        try
        {
            mooon::CUniqId uniq_id(agent_nodes);
            uint64_t uid = uniq_id.get_uniq_id();
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
