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
#include <mooon/net/utils.h>
#include <mooon/sys/utils.h>
#include <mooon/utils/exception.h>
#include <mooon/utils/string_utils.h>

#if MOOON_HAVE_ZOOKEEPER == 1
#include <zookeeper/zookeeper.h>
#endif // MOOON_HAVE_ZOOKEEPER

NET_NAMESPACE_BEGIN
#if MOOON_HAVE_ZOOKEEPER == 1

// 默认zookeeper日志输出到stderr，
// 可以调用zoo_set_log_stream(FILE*)设置输出到文件中
// 还可以调用zoo_set_debug_level(ZooLogLevel)控制日志级别！！！

// 提供基于zookeeper的主备切换接口
//
// 使用示例：
//
// int main(int argc, char* argv[])
// {
//     try
//     {
//         CMyApplication myapp;
//
//         // 由于仅基于zookeeper的ZOO_EPHEMERAL结点实现互斥，没有组合使用ZOO_SEQUENCE，
//         // 本实现要求主备结点的data不为能空也不能相同，最简单的做法是取各自的IP作为data参数。
//
//         // 请注意，在调用connect_zookeeper()或reconnect_zookeeper()后，
//         // 都应当重新调用change_to_master()去竞争成为master，即使此时get_zk_data()仍然取得data。
//         myapp.connect_zookeeper(zk_nodes, zk_path, data, session_timeout_seconds);
//         myapp.work();
//     }
//     catch (mooon::utils::CException& ex)
//     {
//         MYLOG_ERROR("%s\n", ex.str().c_str());
//     }
//     return 0;
// }
//
// class CMyApplication: public CZookeeperHelper
// {
// public:
//     CMyApplication()
//         : _stop(false)
//     {
//     }
//
// private:
//     volatile bool _stop;
//
// public:
//     void work()
//     {
//         become_master();
//
//         while (!_stop)
//         {
//             try
//             {
//                 const NodeState node_state = get_node_state();
//
//                 if (NODE_SLAVE == node_state)
//                 {
//                     break;
//                 }
//                 else if (node_state != NODE_MASTER)
//                 {
//                     // 可能是因为连接不上zookeeper集群的任意节点
//                     mooon::sys::CUtils::millisleep(2000);
//                 }
//                 else
//                 {
//                     // 多sleep一下，以让原来的master足够时间退出，
//                     // 原因是原master不一定及时察觉到与zookeeper网络连接断开等，
//                     // 这样做的目的是为了防止同时存在两个master！！！
//                     mooon::sys::CUtils::millisleep(10000);
//                     do_work();
//                 }
//             }
//             catch (mooon::utils::CException& ex)
//             {
//                 MYLOG_ERROR("%s\n", ex.str().c_str());
//                 if (node_not_exists_exception(ex.errcode()))
//                     break;
//             }
//          }
//     }
//
// private:
//     virtual void on_zookeeper_session_connected(const char* path)
//     {
//         MYLOG_INFO("connect zookeeper[%s] ok: %s\n", get_connected_host().c_str(), path);
//     }
//
//     virtual void on_zookeeper_session_expired(const char *path)
//     {
//         // 当session过期后（expired），要想继续使用则应当重连接，而且不能使用原来的ClientId重连接
//         const NodeState raw_node_state = get_raw_node_state();
//
//         if (raw_node_state != NODE_SLAVE)
//         {
//             _stop = true;
//             MYLOG_ERROR("will exit, zookeeper[%s] expired: %s\n", get_connected_host().c_str(), path);
//
//             // 除了退出进程外，也可以采取调用reconnect_zookeeper()重连接，
//             // 然后再调用change_to_master()竞争成为master，但显然退出重启方式更为简单，不过时效性稍差。
//         }
//         else
//         {
//             try
//             {
//                 reconnect_zookeeper();
//             }
//             catch (mooon::utils::CException& ex)
//             {
//                 MYLOG_ERROR("will exit: %s\n", ex.str().c_str());
//                 _stop = true;
//             }
//         }
//     }
//
//     virtual void on_zookeeper_session_event(int state, const char *path)
//     {
//         MYLOG_INFO("[%s][%s] state: %d\n", get_connected_host().c_str(), path, state);
//     }
//
//     virtual void on_zookeeper_event(int type, int state, const char *path)
//     {
//         MYLOG_INFO("[%s][%s] type: %d, state: %d\n", get_connected_host().c_str(), path, type, state);
//     }
//
// private:
//     void do_work()
//     {
//     }
//
//     void become_master()
//     {
//         while (!_stop)
//         {
//             if (is_connected())
//             {
//                 try
//                 {
//                     change_to_master();
//                     break;
//                 }
//                 catch (mooon::utils::CException& ex)
//                 {
//                     MYLOG_ERROR("change to master failed: %s\n", ex.str().c_str());
//                 }
//             }
//
//             mooon::sys::CUtils::millisleep(2000);
//         }
//     }
// };
class CZookeeperHelper
{
public:
    // 节点状态
    enum NodeState
    {
        NODE_MASTER    = 1, // 明确为master
        NODE_SLAVE     = 2, // 明确为slave
        NODE_UNCERTAIN = 3  // 可能为master，但也可能为slave，此状态时应当去查看data是否为自己，如果是则为master否则不是master了
    };

    static const char* node_state2string(NodeState node_state)
    {
        static const char* node_state_str[] = { "", "master", "slave", "uncertain" };

        if ((node_state < 0) || (node_state > 3))
            return "";
        else
            return node_state_str[node_state];
    }

public:
    static bool node_exists_exception(int errcode) { return ZNODEEXISTS == errcode; }
    static bool node_not_exists_exception(int errcode) { return ZNONODE == errcode; }
    static bool invalid_state_exception(int errcode) { return ZINVALIDSTATE == errcode; }
    static bool connection_loss_exception(int errcode) { return ZCONNECTIONLOSS == errcode; }
    static bool not_empty_exception(int errcode) { return ZNOTEMPTY == errcode; }
    static bool session_expired_exception(int errcode) { return ZSESSIONEXPIRED == errcode; }
    static bool no_authorization_exception(int errcode) { return ZNOAUTH == errcode; }
    static bool invalid_ACL_exception(int errcode) { return ZINVALIDACL == errcode; }

public:
    CZookeeperHelper();
    virtual ~CZookeeperHelper();

    // data用来区分主备，只有主的data存储在zookeeper，
    // get_data()返回进程自己的data，
    // get_zk_data()返回保存在zookeeper的data，
    // 通过对比get_data()和get_zk_data()即可知道自己是不是主！
    const std::string& get_data() const { return _data; }
    const std::string get_zk_data() const throw (utils::CException);

    const std::string& get_zk_path() const { return _zk_path; }
    const std::string& get_zk_nodes() const { return _zk_nodes; }

    // 建立与zookeepr的连接
    // 但请注意本函数返回并不表示和zookeeper连接成功，
    // 只有成员函数on_zookeeper_connected()被回调了才表示连接成功
    //
    // zk_nodes 以逗号分隔的zookeeper节点，如：192.168.31.239:2181,192.168.31.240:2181
    // zk_path zookeeper路径，使用时要求其父路径已存在
    // data 存在zk_path节点上的数据，主备节点设置的data不同相同，比如可以使用IP作为data
    // session_timeout_seconds zookeeper session超时时长，单位为秒，但实际值和zookeeper配置项minSessionTimeout和maxSessionTimeout相关
    //
    // 由于仅基于zookeeper的ZOO_EPHEMERAL结点实现互斥，没有组合使用ZOO_SEQUENCE，
    // 本实现要求主备结点的data不为能空也不能相同，最简单的做法是取各自的IP作为data参数。
    //
    // 请注意，在调用connect_zookeeper()或reconnect_zookeeper()后，
    // 都应当重新调用change_to_master()去竞争成为master，即使此时get_zk_data()仍然取得data。
    void connect_zookeeper(const std::string& zk_nodes, const std::string& zk_path, const std::string& data, int session_timeout_seconds=6) throw (utils::CException);

    // 关闭与zookeeper的连接
    void close_zookeeper() throw (utils::CException);

    // 重新建立与zookeeper的连接，重连接之前会先关闭和释放已建立连接的资源
    //
    // 请注意，在调用connect_zookeeper()或reconnect_zookeeper()后，
    // 都应当重新调用change_to_master()去竞争成为master，即使此时get_zk_data()仍然取得data。
    void reconnect_zookeeper() throw (utils::CException);

    // 得到当前连接的zookeeper host
    std::string get_connected_host() const;
    bool get_connected_host(std::string* ip, uint16_t* port) const;

    // 返回当前是否处于连接状态
    bool is_connected() const;

    // 取得节点状态
    // get_node_state()和get_raw_node_state()的区分：
    // 前者实时取真实的状态，后者直接返回最近一次保存的状态。
    NodeState get_node_state();
    NodeState get_raw_node_state() const { return _node_state; }

    // 取得实际的zookeeper session超时时长，单位为秒
    int get_session_timeout_seconds() const;

    // 切换到master状态
    // 总是只在is_master()返回false的前提下调用change_to_master()
    void change_to_master() throw (utils::CException);

public: // 仅局限于被zk_watcher()调用，其它情况均不应当调用
    void zookeeper_session_connected(const char* path);
    void zookeeper_session_expired(const char *path);
    void zookeeper_session_event(int state, const char *path);
    void zookeeper_event(int type, int state, const char *path);

private:
    // zookeeper session连接成功事件
    virtual void on_zookeeper_session_connected(const char* path) {}

    // zookeeper session过期事件
    // 可以调用reconnect_zookeeper()重连接zookeeper
    // 过期后可以选择调用reconnect_zookeeper()重连接zookeeper。
    // 但请注意，重连接成功后需要重新调用change_to_master()去竞争成为master，
    // 简单点的做法是session过期后退出当前进程，通过重新启动的方式来竞争成为master，
    // 这样只需要在进程启动时，但还未进入工作之前调用change_to_master()去竞争成为master。
    //
    // 当session过期后（expired），要想继续使用则应当重连接，而且不能使用原来的ClientId重连接
    virtual void on_zookeeper_session_expired(const char *path) {}

    // session类zookeeper事件
    virtual void on_zookeeper_session_event(int state, const char *path) {}

    // 非session类zookeeper事件
    virtual void on_zookeeper_event(int type, int state, const char *path) {}

private:
    int _session_timeout_seconds;
    std::string _data;
    std::string _zk_path;
    std::string _zk_nodes;
    zhandle_t* _zk_handle;
    const clientid_t* _zk_clientid;
    NodeState _node_state;
};

inline static void zk_watcher(zhandle_t *zh, int type, int state, const char *path, void *context)
{
    CZookeeperHelper* self = static_cast<CZookeeperHelper*>(context);

    if (type != ZOO_SESSION_EVENT)
    {
        self->zookeeper_event(type, state, path);
    }
    else
    {
        // 1: ZOO_CONNECTING_STATE
        // 2: ZOO_ASSOCIATING_STATE
        // 3: ZOO_CONNECTED_STATE
        // -112: ZOO_EXPIRED_SESSION_STATE
        // 999: NOTCONNECTED_STATE_DEF

        if (ZOO_EXPIRED_SESSION_STATE == state)
        {
            // 需要重新调用zookeeper_init，简单点可以退出当前进程重启
            self->zookeeper_session_expired(path);
        }
        else if (ZOO_CONNECTED_STATE == state)
        {
            // zookeeper_init成功时type为ZOO_SESSION_EVENT，state为ZOO_CONNECTED_STATE
            self->zookeeper_session_connected(path);
        }
        else
        {
            self->zookeeper_session_event(state, path);
        }
    }

    //(void)zoo_set_watcher(zh, zk_watcher);
}

inline CZookeeperHelper::CZookeeperHelper()
    : _session_timeout_seconds(10), _zk_handle(NULL), _zk_clientid(NULL), _node_state(NODE_SLAVE)
{
}

inline CZookeeperHelper::~CZookeeperHelper()
{
    (void)close_zookeeper();
}

inline const std::string CZookeeperHelper::get_zk_data() const throw (utils::CException)
{
    struct Stat stat;
    int datalen = _data.size();
    char data[datalen+1];
    const int errcode = zoo_get(_zk_handle, _zk_path.c_str(), 1, data, &datalen, &stat);

    if (ZOK == errcode)
    {
        return std::string(data, datalen);
    }
    else
    {
        THROW_EXCEPTION(zerror(errcode), errcode);
    }
}

inline void CZookeeperHelper::connect_zookeeper(const std::string& zk_nodes, const std::string& zk_path, const std::string& data, int session_timeout_seconds) throw (utils::CException)
{
    if (zk_nodes.empty())
    {
        THROW_EXCEPTION("parameter[zk_nodes] is empty", EINVAL);
    }
    else if (zk_path.empty())
    {
        THROW_EXCEPTION("parameter[zk_path] is empty", EINVAL);
    }
    else if (data.empty())
    {
        THROW_EXCEPTION("parameter[data] is empty", EINVAL);
    }
    else
    {
        _session_timeout_seconds = session_timeout_seconds*1000;
        _data = data;
        _zk_path = zk_path;
        _zk_nodes = zk_nodes;

        // 当连接不上时，会报如下错误（默认输出到stderr，可通过zoo_set_log_stream(FILE*)输出到文件）：
        // zk retcode=-4, errno=111(Connection refused): server refused to accept the client
        _zk_handle = zookeeper_init(_zk_nodes.c_str(), zk_watcher, _session_timeout_seconds, _zk_clientid, this, 0);
        if (NULL == _zk_handle)
        {
            THROW_EXCEPTION(zerror(errno), errno);
        }
    }
}

inline void CZookeeperHelper::close_zookeeper() throw (utils::CException)
{
    _node_state = NODE_SLAVE;

    if (_zk_handle != NULL)
    {
        int errcode = zookeeper_close(_zk_handle);

        if (errcode != ZOK)
        {
            THROW_EXCEPTION(zerror(errcode), errcode);
        }
        else
        {
            _zk_handle = NULL;
            _zk_clientid = NULL;
            _node_state = NODE_SLAVE;
        }
    }
}

inline void CZookeeperHelper::reconnect_zookeeper() throw (utils::CException)
{
    close_zookeeper();

    _zk_handle = zookeeper_init(_zk_nodes.c_str(), zk_watcher, _session_timeout_seconds, _zk_clientid, this, 0);
    if (NULL == _zk_handle)
    {
        THROW_EXCEPTION(zerror(errno), errno);
    }
}

inline std::string CZookeeperHelper::get_connected_host() const
{
    std::string ip;
    uint16_t port = 0;
    get_connected_host(&ip, &port);
    return utils::CStringUtils::format_string("%s:%u", ip.c_str(), port);
}

inline bool CZookeeperHelper::get_connected_host(std::string* ip, uint16_t* port) const
{
    struct sockaddr_in6 addr_in6;
    socklen_t addr_len = sizeof(addr_in6);
    if (NULL == zookeeper_get_connected_host(_zk_handle, reinterpret_cast<struct sockaddr*>(&addr_in6), &addr_len))
    {
        return false;
    }

    const struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&addr_in6);
    if (AF_INET == addr->sa_family)
    {
        const struct sockaddr_in* addr_in = reinterpret_cast<const struct sockaddr_in*>(addr);
        *ip = net::to_string(addr_in->sin_addr);
        *port = net::CUtils::net2host<uint16_t>(addr_in->sin_port);
    }
    else if (AF_INET6 == addr->sa_family)
    {
        *ip = net::CUtils::ipv6_tostring(addr_in6.sin6_addr.s6_addr32);
        *port = net::CUtils::net2host<uint16_t>(addr_in6.sin6_port);
    }
    else
    {
        return false;
    }

    return true;
}

inline bool CZookeeperHelper::is_connected() const
{
    // 3: ZOO_CONNECTED_STATE
    const int state = zoo_state(_zk_handle);
    return (ZOO_CONNECTED_STATE == state);
}

// 一旦连接断开，状态就不确定，
// 有可能还是master，也有可能其它节点成了master，
// 当不能连接zookeeper时，没有回调来感知，但可以判断连接状态。
inline CZookeeperHelper::NodeState CZookeeperHelper::get_node_state()
{
    if (!is_connected())
    {
        if (NODE_MASTER == _node_state)
            _node_state = NODE_UNCERTAIN;
    }
    else
    {
        if (NODE_UNCERTAIN == _node_state)
        {
            if (get_data() == get_zk_data())
                _node_state = NODE_MASTER;
            else
                _node_state = NODE_SLAVE;
        }
    }

    return _node_state;
}

inline int CZookeeperHelper::get_session_timeout_seconds() const
{
    return zoo_recv_timeout(_zk_handle);
}

inline void CZookeeperHelper::change_to_master() throw (utils::CException)
{
    // ZOO_EPHEMERAL|ZOO_SEQUENCE
    // 调用之前，需要确保_zk_path的父路径已存在
    // (-4)connection loss，比如为zookeeper_init指定了无效的hosts（一个有效的host也没有）
    const int errcode = zoo_create(_zk_handle, _zk_path.c_str(), _data.c_str(), static_cast<int>(_data.size()+1), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
    if (ZOK == errcode)
    {
        _node_state = NODE_MASTER;
    }
    else
    {
        _node_state = NODE_SLAVE;
        THROW_EXCEPTION(zerror(errcode), errcode);
        // ZOK operation completed successfully
        // ZNONODE the parent node does not exist
        // ZNODEEXISTS the node already exists
    }
}

inline void CZookeeperHelper::zookeeper_session_connected(const char* path)
{
    _zk_clientid = zoo_client_id(_zk_handle);
    this->on_zookeeper_session_connected(path);
}

inline void CZookeeperHelper::zookeeper_session_expired(const char* path)
{
    this->on_zookeeper_session_expired(path);
    _node_state = NODE_SLAVE;
}

inline void CZookeeperHelper::zookeeper_session_event(int state, const char *path)
{
    this->on_zookeeper_session_event(state, path);
}

inline void CZookeeperHelper::zookeeper_event(int type, int state, const char *path)
{
    this->on_zookeeper_event(type, state, path);
}

#endif // MOOON_HAVE_ZOOKEEPER == 1
NET_NAMESPACE_END
#endif // MOOON_NET_ZOOKEEPER_HELPER_H
