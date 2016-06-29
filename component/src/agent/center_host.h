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
#ifndef MOOON_AGENT_CENTER_HOST_H
#define MOOON_AGENT_CENTER_HOST_H
#include <mooon/agent/config.h>
AGENT_NAMESPACE_BEGIN

class CCenterHost
{
public:
    CCenterHost(const std::string& ip, uint16_t port);   

    const std::string& get_ip() const
    {
        return _ip;
    }
    
    void set_port(uint16_t port)
    {
        _port = port;
    }
    
    uint16_t get_port() const
    {
        return _port;
    }
    
    void inc_reconn_times()
    {
        ++_reconn_times;
    }

    void reset_reconn_times()
    {
    	_reconn_times = 0;
    }

    uint32_t get_reconn_times() const
    {
    	return _reconn_times;
    }

private:
    std::string _ip;
    uint16_t _port;
    uint32_t _reconn_times;
};

inline bool operator ==(const CCenterHost* center_host, const std::string& ip)
{
	return center_host->get_ip() == ip;
}

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_CENTER_HOST_H
