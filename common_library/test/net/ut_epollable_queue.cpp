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
#include "mooon/sys/thread.h"
#include "mooon/net/epoller.h"
#include "mooon/sys/utils.h"
#include "mooon/utils/array_queue.h"
#include "mooon/sys/datetime_utils.h"
#include "mooon/net/epollable_queue.h"
using namespace mooon;

#define QUEUE_SIZE  10000 // 队列大小
#define LOOP_NUMBER 10000 // 循环次数

// 用来读取队列中数据的线程，将数据从队列中读出，然后输出到标准输出
class CUTEpollableQueueThread: public sys::CThread
{
public:
    CUTEpollableQueueThread(net::CEpollableQueue<utils::CArrayQueue<int> >* queue)
        :_queue(queue)
    {
        uint32_t epoll_size = 10;
        _epoller.create(epoll_size); // 创建Epoll
        _epoller.set_events(queue, EPOLLIN); // 将队列放入Epoll中
    }

    ~CUTEpollableQueueThread()
    {
        _epoller.destroy();
    }

private:
    virtual void run()
    {
        while (!is_stop())
        {
            try
            {
                // Epoll检测队列中是否有数据
                if (0 == _epoller.timed_wait(1000))
                    continue; // 超时则继续等待

                int m = 0;
                if (_queue->pop_front(m)) // 弹出队首数据
                    fprintf(stdout, "<%s> pop %d from queue.\n", sys::CDatetimeUtils::get_current_datetime().c_str(), m);
                else
                    fprintf(stderr, "<%s> get nothing from queue.\n", sys::CDatetimeUtils::get_current_datetime().c_str());
            }
            catch (sys::CSyscallException& ex)
            {
                fprintf(stderr, "CUTEpollableQueueThread exception: %s at %s:%d.\n"
                    ,ex.str().c_str(), ex.file(), ex.line());
            }
        }
    }

private:
    net::CEpoller _epoller;
    net::CEpollableQueue<utils::CArrayQueue<int> >* _queue; // 使用CArrayQueue作为队列容器
};

int main()
{
    try
    {
        uint32_t queue_size = QUEUE_SIZE;
        net::CEpollableQueue<utils::CArrayQueue<int> > queue(queue_size);
        CUTEpollableQueueThread* thread = new CUTEpollableQueueThread(&queue);

        thread->inc_refcount(); // 线程引用计数增一
        thread->start(); // 启动线程

				// 循环往队列中插入数据
        for (int i=1; i<LOOP_NUMBER; ++i)
        {
            if (queue.push_back(i))
                fprintf(stdout, "<%s> push %d to queue.\n", sys::CDatetimeUtils::get_current_datetime().c_str(), i);
            else
                fprintf(stderr, "<%s> failed to push %d to queue.\n", sys::CDatetimeUtils::get_current_datetime().c_str(), i);

						// 让线程sleep一秒钟
            sys::CUtils::millisleep(1000);
        }

        thread->stop(); // 停止线程
        thread->dec_refcount(); // 线程引用计数减一，这个必须在thread->stop();调用之后
    }
    catch (sys::CSyscallException& ex)
    {
    		// 异常处理
        fprintf(stderr, "main exception: %s at %s:%d.\n", ex.str().c_str(), ex.file(), ex.line());
    }

    return 0;
}
