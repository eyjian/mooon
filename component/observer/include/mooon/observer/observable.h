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
#ifndef MOOON_OBSERVER_OBSERVABLE_H
#define MOOON_OBSERVER_OBSERVABLE_H
#include <mooon/observer/data_reporter.h>
OBSERVER_NAMESPACE_BEGIN

/***
  * 可观察者接口
  */
class CALLBACK_INTERFACE IObservable
{
public:
    /** 虚拟析构函数，仅为应付编译器告警 */
    virtual ~IObservable() {}

    /***
      * 上报通知方法，由Observer定时回调
      * @data_reporter: 数据上报器
      */
	virtual void on_report(IDataReporter* data_reporter, const std::string& current_datetime) = 0;
};

OBSERVER_NAMESPACE_END
#endif // MOOON_OBSERVER_OBSERVABLE_H
