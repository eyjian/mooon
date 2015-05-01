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
#ifndef MOOON_NET_LISTEN_MANAGER_H
#define MOOON_NET_LISTEN_MANAGER_H
#include "mooon/net/ip_address.h"
NET_NAMESPACE_BEGIN

/***
  * 监听管理者模板类
  */
template <class ListenClass>
class CListenManager
{
public:
    CListenManager()
        :_listener_count(0)
        ,_listener_array(NULL)
    {
    }

    /***
      * 新增IP端口对，这里不做重复检查，所以调用前必须保证IP和端口不重复，另外
      * 应当在调用create之前调用add，以设置好需要监听的地址和端口号，只要IP+port不重复即可
      * @ip: 监听IP地址
      * @port: 监听端口号
      * 不会抛出任何异常
      */
    void add(const ip_address_t& ip, port_t port)
    {
        ip_port_pair_t ip_port_pair;

        ip_port_pair.first = ip;
        ip_port_pair.second = port;

        _ip_port_array.push_back(ip_port_pair);
    }

    /***
      * 启动在所有IP和端口对上的监听
      * @exception: 如果出错，则抛出CSyscallException异常
      */
    void create(bool nonblock=true)
    {
        _listener_array = new ListenClass[_ip_port_array.size()];

        for (ip_port_pair_array_t::size_type i=0; i<_ip_port_array.size(); ++i)
        {
            try
            {                
                _listener_array[i].listen(_ip_port_array[i].first, _ip_port_array[i].second, nonblock);
                ++_listener_count;
            }
            catch (...)
            {
                destroy();
                throw;
            }
        }
    }

    /***
      * 销毁和关闭在所有端口上的监听
      * 不会抛出任何异常
      */
    void destroy()
    {
        uint16_t listen_counter = _listener_count;
        for (uint16_t i=listen_counter; i>0; --i)
        {
            _listener_array[i-1].close();
            --_listener_count;
        }

        delete []_listener_array;
        _listener_array = NULL;
    }

    /** 得到监听者个数 */
    uint16_t get_listener_count() const { return _listener_count; }

    /** 得到指向监听者对象数组指针 */
    ListenClass* get_listener_array() const { return _listener_array; }
    
private:
    uint16_t _listener_count;
    ListenClass* _listener_array;
    ip_port_pair_array_t _ip_port_array;
};

NET_NAMESPACE_END
#endif // MOOON_NET_LISTEN_MANAGER_H
