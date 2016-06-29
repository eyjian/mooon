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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_NET_EPOLLER_H
#define MOOON_NET_EPOLLER_H
#include <sys/epoll.h>
#include "mooon/net/sensor.h"
#include "mooon/net/epollable.h"
NET_NAMESPACE_BEGIN

/***
  * Epoll操作封装类
  */
class CEpoller
{
public:
    /***
      * 构造一个Epoll对象
      * 不会抛出任何异常
      */
    CEpoller();
    ~CEpoller();

    /***
      * 创建Epoll，进行初始化
      * @epoll_size: 建议性Epoll大小
      * @exception: 如果出错，抛出CSyscallException异常
      */
    void create(uint32_t epoll_size);

    /***
      * 销毁已经创建的Epoll
      * 不会抛出任何异常
      */
    void destroy();

    /***
      * 以超时方式等待Epoll有事件，如果指定的时间内无事件，则超时返回
      * @milliseconds: 最长等待的毫秒数，总是保证等待这个时长，即使被中断
      * @return: 如果在超时时间内，有事件，则返回有事件的对象个数，
      *          否则返回0表示已经超时了
      * @exception: 如果出错，抛出CSyscallException异常
      */
    int timed_wait(uint32_t milliseconds);

    /***
      * 将一个可Epoll的对象注册到Epoll监控中
      * @epollable: 指向可Epoll对象的指针
      * @events: 需要监控的Epoll事件，取值可以为: EPOLLIN和EPOLLOUT等，
      *          具体请查看Epoll系统调用说明手册
      *          通常不需要显示设置EPOLLERR和EPOLLHUP两个事件，因为它们总是
      *          会被自动设置
      * @force: 是否强制以新增方式加入
      * @exception: 如果出错，抛出CSyscallException异常
      */
    void set_events(CEpollable* epollable, int events, bool force=false);

    /***
      * 将一个可Epoll对象从Epoll中删除
      * @epollable: 指向可Epoll对象的指针
      * @exception: 如果出错，抛出CSyscallException异常
      */
    void del_events(CEpollable* epollable);

    /***
      * 根据编号得到一个指向可Epoll对象的指针
      * @index: 编号，请注意index必须在timed_wait成功的返回值范围内
      * @return: 返回一个指向可Epoll对象的指针
      */
    CEpollable* get(uint32_t index) const { return (CEpollable *)_events[index].data.ptr; }

    /***
      * 根据编号得到触发的Epoll事件
      * @index: 编号，请注意index必须在timed_wait成功的返回值范围内
      * @return: 返回发生的Epoll事件
      */
    uint32_t get_events(uint32_t index) const { return _events[index].events; }

    /***
      * 唤醒Epoll
      */
    void wakeup();

private:
    int _epfd;
    CSensor _sensor;
    uint32_t _epoll_size;
    uint32_t _max_events;
    struct epoll_event* _events;     
};

NET_NAMESPACE_END
#endif // MOOON_NET_EPOLLER_H
