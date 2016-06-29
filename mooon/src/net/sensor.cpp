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
#include "net/sensor.h"
NET_NAMESPACE_BEGIN

CSensor::CSensor()
{
    _pipe_fd[0] = -1;
    _pipe_fd[1] = -1;
    _finger[0] = '\0';
}

void CSensor::create()
{
    if (-1 == pipe(_pipe_fd))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "pipe");

    set_fd(_pipe_fd[0]);
}

void CSensor::close()
{        
    if (_pipe_fd[0] != -1)
    {
        before_close();

        close_fd(_pipe_fd[0]);
        close_fd(_pipe_fd[1]);

        _pipe_fd[0] = -1;
        _pipe_fd[1] = -1;

        set_fd(-1);
    }    
}

void CSensor::touch()
{
    char finger = 'x';
    if (-1 == write(_pipe_fd[1], &finger, sizeof(finger)))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "write");
}

void CSensor::feel(uint16_t bytes)
{
    if (-1 == read(_pipe_fd[0], _finger, bytes<sizeof(_finger)? bytes: sizeof(_finger)))
        THROW_SYSCALL_EXCEPTION(NULL, errno, "read");
}

epoll_event_t CSensor::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{    
    feel(1024);
    return epoll_none;    
}

NET_NAMESPACE_END
