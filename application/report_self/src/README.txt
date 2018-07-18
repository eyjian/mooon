ReportSelfServer的结果存储在MySQL表t_program_deployment中，
在运行之前需要创建好此表，而数据库名等信息通过命令行参数指定。

表名t_program_deployment结构：
f_md5,f_ip,f_user,f_shortname,f_dirpath,f_lasttime,f_interval,f_pid,f_vsz,f_rss,f_firsttime

DROP TABLE IF EXISTS t_program_deployment;
CREATE TABLE t_program_deployment (
    f_id BIGINT NOT NULL AUTO_INCREMENT, # 自增ID
    f_md5 VARCHAR(32) NOT NULL PRIMARY KEY, # f_full_cmdline的MD5值
    f_ip VARCHAR(32) NOT NULL, # 进程所在机器的IP
    f_user VARCHAR(24) NOT NULL, # 进程的当前系统用户名
    f_shortname VARCHAR(64) NOT NULL, # 进程的短名称
    f_dirpath VARCHAR(256) NOT NULL, # 程序所在目录
    f_firsttime DATETIME NOT NULL, # 最近一次上报时间
    f_lasttime DATETIME NOT NULL, # 最近一次上报时间
    f_interval INT UNSIGNED NOT NULL DEFAULT 0, # 上报间隔，单位为秒
    f_pid INT UNSIGNED NOT NULL DEFAULT 0, # 进程的进程ID
    f_vsz BIGINT UNSIGNED NOT NULL DEFAULT 0, # 进程所占的虚拟内存
    f_rss BIGINT UNSIGNED NOT NULL DEFAULT 0, # 进程所占的物理内存
    f_state TINYINT DEFAULT 0,
    f_memo VARCHAR(256),
    INDEX idx_id (f_id),
    INDEX idx_md5 (f_md5),
    INDEX idx_ip (f_ip),
    INDEX idx_pid (f_pid),
    INDEX idx_shortname (f_shortname),
    INDEX idx_firsttime (f_firsttime),
    INDEX idx_lasttime (f_lasttime),
    INDEX idx_state (f_state)
);
