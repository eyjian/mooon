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
#include <mooon/sys/thread.h>
#include <mooon/sys/utils.h>
#include <mooon/sys/event_queue.h>
#include <mooon/utils/array_queue.h>
#include <mooon/sys/datetime_utils.h>
MOOON_NAMESPACE_USE

// 定义处理消息线程，由主线程向它发消息
class CMyThread: public sys::CThread
{
public:
    // 设置队列大小为100
    // pop等待时长为10000毫秒
    // push等待时长为0毫秒
    CMyThread()
        :_queue(100, 3000, 0)
    {
    }

    void push_message(int m)
    {
        if (_queue.push_back(m))
            printf("push %d SUCCESS by thread %u\n", m, sys::CThread::get_current_thread_id());
        else
            printf("push %d FAILURE by thread %u\n", m, sys::CThread::get_current_thread_id());
    }
    
private:
    virtual void run()
    {
        for (;;)
        {
            // 加判断，以保证所有消息都处理完才退出
            if (is_stop() && _queue.is_empty()) break;
            
            int m;
            printf("before pop ==> %s\n", sys::CDatetimeUtils::get_current_time().c_str());
            if (_queue.pop_front(m))
                printf("pop %d ==> %s\n", m, sys::CDatetimeUtils::get_current_time().c_str());
            else
                printf("pop NONE ==> %s\n", sys::CDatetimeUtils::get_current_time().c_str());
        }
    }

private:
    // 使用整数类型的数组队列
    sys::CEventQueue<utils::CArrayQueue<int> > _queue;
};

int main()
{
    CMyThread* thread = new CMyThread;
    // 使用引用计数帮助类，以协助自动销毁thread
    sys::CRefCountHelper<CMyThread> ch(thread);

    try
    {
        thread->start(); // 启动线程

        // 给线程发消息
        for (int i=0; i<10; ++i)
        {
            sys::CUtils::millisleep(2000);
            thread->push_message(i);
        }        

        // 让CMyThread超时
        sys::CUtils::millisleep(3000);

        // 停止线程
        thread->stop();
    }
    catch (sys::CSyscallException& ex)
    {
        printf("Main exception: %s at %s:%d\n", ex.str().c_str(), ex.file(), ex.line());
    }
    
    return 0;
}
/*
运行上面的代码，输出的内容大致（线程号和时间会不同，另外pop和push的顺序可能会有些不同，因为两线程为并行的，但输出的最后一行内容都应当为pop NONE）如下:
pop 0 ==> 21:06:59
before pop ==> 21:06:59
push 0 SUCCESS by thread 782374640
push 1 SUCCESS by thread 782374640
pop 1 ==> 21:07:01
before pop ==> 21:07:01
push 2 SUCCESS by thread 782374640
pop 2 ==> 21:07:03
before pop ==> 21:07:03
push 3 SUCCESS by thread 782374640
pop 3 ==> 21:07:05
before pop ==> 21:07:05
push 4 SUCCESS by thread 782374640
pop 4 ==> 21:07:07
before pop ==> 21:07:07
push 5 SUCCESS by thread 782374640
pop 5 ==> 21:07:09
before pop ==> 21:07:09
push 6 SUCCESS by thread 782374640
pop 6 ==> 21:07:11
before pop ==> 21:07:11
push 7 SUCCESS by thread 782374640
pop 7 ==> 21:07:13
before pop ==> 21:07:13
push 8 SUCCESS by thread 782374640
pop 8 ==> 21:07:15
before pop ==> 21:07:15
push 9 SUCCESS by thread 782374640
pop 9 ==> 21:07:17
before pop ==> 21:07:17
pop NONE ==> 21:07:20
*/
