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
#ifndef MOOON_SERVER_CONTEXT_H
#define MOOON_SERVER_CONTEXT_H
#include <sys/log.h>
#include <sys/thread_pool.h>
#include <net/listen_manager.h>
#include "listener.h"
#include "work_thread.h"
#include "server/server.h"
SERVER_NAMESPACE_BEGIN

class CContext
{
public:
    ~CContext();
    CContext(IConfig* config, IFactory* factory);

    bool create();
    void destroy();

public:
    IConfig* get_config() const { return _config; }
    IFactory* get_factory() const { return _factory; }
    CWorkThread* get_thread(uint16_t thread_index);
    CWorkThread* get_thread(uint16_t thread_index) const;

private:
    bool IgnorePipeSignal();
    bool create_listen_manager();
    bool create_thread_pool(net::CListenManager<CListener>* listen_manager);
    
private:
    IConfig* _config;
    IFactory* _factory;   
    sys::CThreadPool<CWorkThread> _thread_pool;
    net::CListenManager<CListener> _listen_manager;    
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_H
