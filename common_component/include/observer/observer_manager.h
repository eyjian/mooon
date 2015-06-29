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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_OBSERVER_MANAGER_H
#define MOOON_OBSERVER_MANAGER_H
#include <mooon/observer/observable.h>
#include <mooon/observer/data_reporter.h>
#include <mooon/sys/log.h>
OBSERVER_NAMESPACE_BEGIN

/***
  * 观察者管理器
  */
class IObserverManager
{
public:
    /** 虚拟析构函数，仅为应付编译器告警 */
    virtual ~IObserverManager() {}

	/***
      * 向Observer注册可观察者
      * @observee: 被注册的可观察者
      */
	virtual void register_observee(IObservable* observee) = 0;

    /***
      * 注销可观察者
      * @observee: 需要被注销的可观察者
      */
    virtual void deregister_objservee(IObservable* observee) = 0;
};

//////////////////////////////////////////////////////////////////////////

/** observer日志器 */
extern sys::ILogger* logger;

/** 销毁观察者管理器 */
extern void destroy();

/** 获得观察者管理器 */
extern IObserverManager* get();

/***
  * 创建观察者管理器
  * @logger: 日志器
  * @data_reporter: 数据上报器
  * @report_frequency_seconds: 数据上报频率(单位: 秒)
  */
extern IObserverManager* create(IDataReporter* data_reporter, uint16_t report_frequency_seconds);

OBSERVER_NAMESPACE_END
#endif // MOOON_OBSERVER_MANAGER_H
