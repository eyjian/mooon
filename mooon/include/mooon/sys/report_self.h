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
#ifndef MOOON_APPLICATION_REPORT_SELF_H
#define MOOON_APPLICATION_REPORT_SELF_H
#include <mooon/sys/log.h>
SYS_NAMESPACE_BEGIN

// 注意：
// 需要配合应用ReportSelf使用，
// report_self.h为ReportSelf的客户端

// 请使用mooon::sys::g_logger记录日志，
// 即使用MYLOG_INFO系列宏写日志，
// 以方便可查看到运行信息

// 取得当前进程的虚拟内存和物理内存
extern std::pair<uint64_t, uint64_t> get_self_memory();

// 停止上报，并结束上报线程
extern void stop_report_self();

// 启动上报，会创建一专门的上报线程
// conffile 配置文件，如果值为空，则什么也不做，直接返回true
// report_interval_seconds 上报间隔，默认每2小时上报一次
//
// 返回值：启动成功，或conffile参数值为空字符串
extern bool start_report_self(const std::string& conffile="/etc/mooon_report_self.conf", uint32_t report_interval_seconds=3600);

/*
 * 配置文件要求结构：
 *
{
    "ethX":"本机IP所在网卡名",
    "report_self_servers":"ReportServer的IP端口列表，逗号分隔，如：192.168.31.11:7110,192.168.31.12:7110",
}
 */

SYS_NAMESPACE_END
#endif // MOOON_APPLICATION_REPORT_SELF_H
