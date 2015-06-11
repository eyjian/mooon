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
#include <list>
#include <mooon/sys/datetime_utils.h>
#include <mooon/sys/event_queue.h>
#include <mooon/sys/thread_engine.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/array_queue.h>
#include <stdlib.h>
MOOON_NAMESPACE_USE

static int g_times = 1000000;

class CMyThread
{
public:
    CMyThread()
        : _stop(false), _queue(10000, 1000, 2000)
    {
    }

    bool push_message(int m, int index)
    {
        return _queue.push_back(m);
    }

    void stop()
    {
        _stop = true;
    }

    void run(int index)
    {
        int times = 0;
        int total = g_times;

        while (true)
        {
            int m = -1;

            if (!_queue.pop_front(m))
            {
                if (!_stop)
                {
                    printf("thread[%d] pop timeout, but not exit\n", index);
                }
                else
                {
                    printf("thread[%d] pop timeout, and exit\n", index);
                    break;
                }
            }
            else
            {
                ++times;

                if ((0 == times%10) && (times < g_times))
                {
                    if (push_message(m, index))
                        ++total;
                    else
                        printf("[SELF] push %d to queue[%u] FAILURE by thread[%d]\n", m, _queue.size(), index);
                }
            }
        }

        printf("times of thread[%d]=%d\n", index, times);
    }

private:
    volatile bool _stop;
    sys::CEventQueue<utils::CArrayQueue<int> > _queue;
    //sys::CEventQueue<sys::CEventQueueAdapterForList<int> > _queue;
};

int main(int argc, char* argv[])
{
    int i;
    const int num_threads = (1 == argc)? 6: atoi(argv[1])+1;
    CMyThread* my_thread[num_threads];
    sys::CThreadEngine* engine[num_threads];

    for (i=0; i<num_threads; ++i)
    {
        my_thread[i] = new CMyThread;
        engine[i] = new sys::CThreadEngine(sys::bind(&CMyThread::run, my_thread[i], i));
    }

    for (int j=0; j<g_times; ++j)
    {
        for (i=0; i<num_threads; ++i)
            if (!my_thread[i]->push_message(j, i))
                printf("[MAIN] push %d to thread[%d] FAILURE\n", j, i);
    }

    for (i=0; i<num_threads; ++i)
    {
        my_thread[i]->stop();
        engine[i]->join();
        delete my_thread[i];
        delete engine[i];
    }

    return 0;
}
