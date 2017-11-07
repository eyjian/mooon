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
#include <mooon/sys/config.h>
SYS_NAMESPACE_BEGIN

// 请使用mooon::sys::g_logger记录日志，
// 即使用MYLOG_INFO系列宏写日志，
// 以方便可查看到运行信息

// 停止上报，并结束上报线程
extern void stop_report_self();

// 启动上报，会创建一专门的上报线程
// conffile 配置文件
// report_interval_seconds 上报间隔，默认一小时上报一次
extern bool start_report_self(const std::string& conffile="/etc/mooon_report_self.conf", uint32_t report_interval_seconds=3600);

/*
 * 配置文件要求结构：
 *
{
    "ethX":"本机IP所在网卡名",
    "db_ip":"MySQL的IP",
    "db_port":MySQL的端口,
    "db_user":"MySQL的用户名",
    "db_password":"MySQL的用户密码",
    "db_name":"MySQL的DB名",
    "table_name":"MySQL的表名"
}
 */

/*
 * 表table_name的结构（假设表名为t_program_deployment）：
 *
DROP TABLE IF EXISTS t_program_deployment;
CREATE TABLE t_program_deployment (
    f_id BIGINT NOT NULL AUTO_INCREMENT, # 自增ID
    f_md5 VARCHAR(32) NOT NULL PRIMARY KEY, # f_full_cmdline的MD5值
    f_ip VARCHAR(24) NOT NULL, # 进程所在机器的IP
    f_user VARCHAR(16) NOT NULL, # 进程的当前系统用户名
    f_shortname VARCHAR(64) NOT NULL, # 进程的短名称
    f_dirpath VARCHAR(256) NOT NULL, # 程序所在目录
    f_full_cmdline VARCHAR(8192) NOT NULL, # 进程的完全命令行
    f_lasttime DATETIME NOT NULL, # 最近一次上报时间
    f_interval INT UNSIGNED NOT NULL DEFAULT 0, # 上报间隔，单位为秒
    f_pid INT UNSIGNED NOT NULL DEFAULT 0, # 进程的进程ID
    f_vsz INT UNSIGNED NOT NULL DEFAULT 0, # 进程所占的虚拟内存
    f_vss INT UNSIGNED NOT NULL DEFAULT 0, # 进程所占的物理内存
    f_state TINYINT DEFAULT 0,
    f_memo VARCHAR(256),
    INDEX idx_id (f_id),
    INDEX idx_ip (f_ip),
    INDEX idx_pid (f_pid),
    INDEX idx_shortname (f_shortname),
    INDEX idx_lasttime (f_lasttime),
    INDEX idx_state (f_state)
);
 */

SYS_NAMESPACE_END
