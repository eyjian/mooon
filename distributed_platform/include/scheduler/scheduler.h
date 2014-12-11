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
#ifndef MOOON_SCHEDULER_H
#define MOOON_SCHEDULER_H
#include <scheduler/error_code.h>
#include <scheduler/message.h>
SCHED_NAMESPACE_BEGIN

/***
  * 加载一个Service需要提供的信息
  */
typedef struct TServiceInfo
{
    std::string service_name; /** 服务名，要求其共享库名为lib+service_name+.so */
	uint32_t service_id;      /** Service ID，取值不能超过SERVICE_ID_MAX */
	uint32_t service_version; /** Service的版本，在线升级需要用到，两个版本会同时服务一段时间 */
	uint16_t num_threads;     /** 独占的线程个数 */
	bool is_process_mode;     /** 是否为进程模型 */

	TServiceInfo()
	 :service_id(0)
	 ,service_version(0)
	 ,num_threads(0)
	 ,is_process_mode(false)
	{
	}

	TServiceInfo(const TServiceInfo& other)
	 :service_id(other.service_id)
	 ,service_version(other.service_version)
	 ,num_threads(other.num_threads)
	 ,is_process_mode(other.is_process_mode)
	{
	}

	std::string to_string() const;
}service_info_t;

/***
  * 调度器接口
  */
class IScheduler
{
public:
	virtual ~IScheduler() {}

    /***
      * 向调度器提交一个消息
      */
    virtual int submit_message(const net::common_message_header* message) = 0;
    
    /***
      * 请求调度器加载一个Service
      */
    virtual int load_service(const TServiceInfo& service_info) = 0;
    
    /***
      * 请求调度器卸载一个Service
      */
    virtual int unload_service(uint32_t service_id, uint32_t service_version) = 0;
};

/***
  * 日志器，所以分发器实例共享
  * 如需要记录日志，则在调用create之前，应当先设置好日志器
  */
extern sys::ILogger* logger;

extern IScheduler* create(dispatcher::IDispatcher* dispatcher);
extern void destroy(IScheduler* scheduler);

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_H
