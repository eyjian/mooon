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
 * Author: eyjian@qq.com eyjian@gmail.com
 */
#ifndef MOOON_NET_SENSOR_H
#define MOOON_NET_SENSOR_H
#include "mooon/net/epollable.h"
NET_NAMESPACE_BEGIN

/***
  * 感应器
  */
class CSensor: public CEpollable
{
public:
    CSensor();

    /***
      * 创建感应器
      * @exception 如果出错则抛出CSyscallException异常
      */
    void create();

    /***
      * 重写CEpollable的close方法
      * 关闭感应器
      */
    virtual void close();

    /***
      * 触摸
      * @exception 如果出错则抛出CSyscallException异常
      */
    void touch();

    /***
      * 感觉
      * @exception 如果出错则抛出CSyscallException异常
      */
    void feel(uint16_t bytes=1);

private:
    virtual epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr);

private:
    int _pipe_fd[2];
    char _finger[1024];
};

NET_NAMESPACE_END
#endif // MOOON_NET_SENSOR_H
