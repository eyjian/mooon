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
MOOON_NAMESPACE_BEGIN

/** 得到epoll大小 */
uint32_t CReceiverConfig::get_epoll_size() const
{

}

/** 得到epool等待超时毫秒数 */
uint32_t CReceiverConfig::get_epoll_timeout() const;
{

}

/** 得到框架的工作线程个数 */
uint16_t CReceiverConfig::get_thread_number() const;
{

}

/** 得到连接池大小 */
uint32_t CReceiverConfig::get_connection_pool_size() const;
{

}

/** 连接超时秒数 */
uint32_t CReceiverConfig::get_connection_timeout_seconds() const;
{

}

/** 得到监听参数 */    
const net::ip_port_pair_array_t& CReceiverConfig::get_listen_parameter() const;
{

}

MOOON_NAMESPACE_END
#endif // MOOON_RECEIVER_H
