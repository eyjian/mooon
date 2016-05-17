/**
 * Copyright (c) 2016, Jian Yi <eyjian at gmail dot com>
 *
 * All rights reserved.
 *
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
 */
#ifndef MOOON_NET_ZOOKEEPER_HELPER_H
#define MOOON_NET_ZOOKEEPER_HELPER_H
#include <mooon/net/config.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/exception.h>

#if HAVE_ZOOKEEPER == 1
#include <zookeeper/zookeeper.h>
#endif // HAVE_ZOOKEEPER

NET_NAMESPACE_BEGIN
#if HAVE_ZOOKEEPER == 1

// 提供基于zookeeper的主备切换接口
//
// 使用示例：
// class CMyApplication: public CZookeeperHelper
// {
// private:
//     virtual void on_zookeeper_connected(const std::string& path);
//     virtual void on_zookeeper_expired();
//     virtual void on_zookeeper_exception(int errcode, const char* errmsg);
// };
class CZookeeperHelper
{
public:
    CZookeeperHelper();
    virtual ~CZookeeperHelper();
    const std::string& get_data() const { return _data; }
    const std::string& get_zk_path() const { return _zk_path; }
    const std::string& get_zk_nodes() const { return _zk_nodes; }

    // 建立与zookeepr的连接
    // 但请注意本函数返回并不表示和zookeeper连接成功，
    // 只有成员函数on_zookeeper_connected()被回调了才表示连接成功
    //
    // zk_nodes 以逗号分隔的zookeeper节点，如：192.168.31.239:2181,192.168.31.240:2181
    // zk_path zookeeper路径，使用时要求其父路径已存在
    // data 存在zk_path节点上的数据，为空表示不存储数据
    void connect_zookeeper(const std::string& zk_nodes, const std::string& zk_path, const std::string& data=std::string("")) throw (utils::CException);

    // 关闭与zookeeper的连接
    void close_zookeeper() throw (utils::CException);

    // 重新建立与zookeeper的连接，重连接之前会先关闭和释放已建立连接的资源
    void reconnect_zookeeper() throw (utils::CException);

    // 返回当前是否为master状态
    bool is_master() const { return _is_master; }

    // 切换到master状态
    // 总是只在is_master()返回false的前提下调用change_to_master()
    void change_to_master() throw (utils::CException);

public: // 仅局限于被zk_watcher()调用，其它情况均不应当调用
    void zookeeper_connected(const std::string& path);
    void zookeeper_expired();

private: // 子类可以和需要重写的函数
    virtual void on_zookeeper_connected(const std::string& path) {}

    // zookeeper会话过期事件
    // 可以调用reconnect_zookeeper()重连接zookeeper
    virtual void on_zookeeper_expired() {}

    // zookeeper异常，可以在日志中记录相应的错误信息
    virtual void on_zookeeper_exception(int errcode, const char* errmsg) {}

private:
    std::string _data;
    std::string _zk_path;
    std::string _zk_nodes;
    clientid_t* _zk_clientid;
    zhandle_t* _zk_handle;
    volatile bool _zk_connected;
    volatile bool _is_master;
};

inline CZookeeperHelper::CZookeeperHelper()
    : _zk_clientid(NULL), _zk_handle(NULL), _zk_connected(false), _is_master(false)
{
}

inline CZookeeperHelper::~CZookeeperHelper()
{
    (void)close_zookeeper();
}

inline void zk_watcher(zhandle_t *zh, int type, int state, const char *path, void *context)
{
    CZookeeperHelper* self = static_cast<CZookeeperHelper*>(context);

    if (ZOO_SESSION_EVENT == type)
    {
        if (ZOO_EXPIRED_SESSION_STATE == state)
        {
            // 需要重新调用zookeeper_init，简单点可以退出当前进程重启
            self->zookeeper_expired();
        }
        else if (ZOO_CONNECTED_STATE == state)
        {
            // zookeeper_init成功时type为ZOO_SESSION_EVENT，state为ZOO_CONNECTED_STATE
            self->zookeeper_connected(path);
        }
    }
}

inline void CZookeeperHelper::connect_zookeeper(const std::string& zk_nodes, const std::string& zk_path, const std::string& data) throw (utils::CException)
{
    _data = data;
    _zk_path = zk_path;
    _zk_nodes = zk_nodes;
    _zk_handle = zookeeper_init(_zk_nodes.c_str(), zk_watcher, 5000, _zk_clientid, this, 0);
    if (NULL == _zhandle)
    {
        THROW_EXCEPTION(zerror(errno), errno);
    }
}

inline void CZookeeperHelper::close_zookeeper() throw (utils::CException)
{
    if (_zhandle != NULL)
    {
        int errcode = zookeeper_close(_zk_handle);
        if (errcode != ZOK)
        {
            THROW_EXCEPTION(zerror(errcode), errcode);
        }
        else
        {
            _zhandle = NULL;
            _zk_clientid = NULL;
            _data.clear();
            _zk_path.clear();
            _zk_nodes.clear();
        }
    }
}

inline void CZookeeperHelper::reconnect_zookeeper() throw (utils::CException)
{
    close_zookeeper();

    _zk_handle = zookeeper_init(_zk_nodes.c_str(), zk_watcher, 5000, _zk_clientid, this, 0);
    if (NULL == _zhandle)
    {
        THROW_EXCEPTION(zerror(errno), errno);
    }
}

inline void CZookeeperHelper::change_to_master() throw (utils::CException)
{
    int errcode;

    // ZOO_EPHEMERAL|ZOO_SEQUENCE
    // 调用之前，需要确保_zk_path的父路径已存在
    if (_data.empty())
        errcode = zoo_create(_zk_handle, _zk_path.c_str(), NULL, 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
    else
        errcode = zoo_create(_zk_handle, _zk_path.c_str(), _data.c_str(), _data.size()+1, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);

    // (-4)connection loss，比如为zookeeper_init指定了无效的hosts（一个有效的host也没有）
    if (ZOK == errcode)
    {
        _is_master = true;
    }
    else
    {
        _is_master = false;
        THROW_EXCEPTION(zerror(errcode), errcode);
    }
}

inline void CZookeeperHelper::zookeeper_connected(const std::string& path)
{
    char data[2048];
    int datalen = static_cast<int>(sizeof(data));
    struct Stat stat; // zookeeper定义的Stat

    int errcode = zoo_get(_zk_handle, _zk_path.c_str(), 1, data, &datalen, &stat);
    if (errcode != ZOK)
    {
        this->on_zookeeper_exception(errcode, zerror(errcode));
    }
    else
    {
        _zk_clientid = zoo_client_id(_zk_handle);
        _zk_connected = true;
        this->on_zookeeper_connected(path);
    }
}

inline void CZookeeperHelper::zookeeper_expired()
{
    _zk_connected = false;
    _is_master = false;
    this->on_zookeeper_expired();
}

#endif // HAVE_ZOOKEEPER == 1
NET_NAMESPACE_END
#endif // MOOON_NET_ZOOKEEPER_HELPER_H
