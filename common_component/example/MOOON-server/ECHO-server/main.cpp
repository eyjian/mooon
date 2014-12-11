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
#include <util/string_util.h>
#include <sys/main_template.h>
#include "config_impl.h"
#include "factory_impl.h"

/***
  * 使用main函数模板，需要实现sys::IMainHelper接口
  */
class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper();

private:        
    virtual bool init(int argc, char* argv[]);    
    virtual void fini();
    virtual int get_exit_signal() const { return SIGUSR1; }

private:
    uint16_t get_listen_port(int argc, char* argv[]);

private:
    server::server_t _server;
    CConfigImpl _config_impl;
    CFactoryImpl _factory_impl;
};

/***
  * 可带一个端口号参数，也可不带任何参数，默认端口号为2012
  */
int main(int argc, char* argv[])
{
    CMainHelper main_helper;
    return sys::main_template(&main_helper, argc, argv);
}

CMainHelper::CMainHelper()
    :_server(NULL)
{
}

bool CMainHelper::init(int argc, char* argv[])
{
    uint16_t port = get_listen_port(argc, argv);
    _config_impl.init(port);

    // 创建一个MOOON-server组件实例
    _server = server::create(&_config_impl, &_factory_impl);
    return _server != NULL;
}

void CMainHelper::fini()
{
    if (_server != NULL)
    {
        // 销毁MOOON-server组件实例
        server::destroy(_server);
        _server = NULL;
    }
}

uint16_t CMainHelper::get_listen_port(int argc, char* argv[])
{
    uint16_t port = 0;

    if (argc > 1)
    {
        (void)util::CStringUtil::string2int(argv[1], port);
    }

    return port;
}
