#!/bin/sh
# http://code.google.com/p/mooon
# Created by yijian on 2012/7/23
#
# 推荐放到crontab中，如每2秒运行一次：
# */2 * * * * /usr/local/bin/process_monitor.sh /usr/sbin/rinetd /usr/sbin/rinetd
#
# 进程监控脚本，当指定进程不存在时，执行重启脚本将它拉起
# 特色：
# 1.互斥不仅依据监控脚本文件名，而且包含了它的命令行参数，只有整体相同时互斥才生效
# 2.对于被监控的进程，可以只指定进程名，也可以包含命令行参数
# 3.不管是监控脚本还是被监控进程，总是只针对属于当前用户下的进程

# 需要指定个数的命令行参数
# 参数1：被监控的进程名（可以包含命令行参数）
# 参数2：重启被监控进程的脚本
if test $# -ne 2; then
	printf "\033[1;33musage: $0 process_cmdline restart_script\033[m\n"
    printf "\033[1;33mexample: /usr/local/bin/process_monitor.sh \"/usr/sbin/rinetd\" \"/usr/sbin/rinetd\"\033[m\n"
    printf "\033[1;33mplease install process_monitor.sh into crontab by \"*/N * * * *\"\033[m\n"
	exit 1
fi

process_cmdline="$1" # 需要监控的进程名，或完整的命令行，也可以为部分命令行
restart_script="$2"  # 用来重启进程的脚本，要求具有可执行权限
cur_user=`whoami`    # 执行本监控脚本的用户名

# 取指定网卡上的IP地址
#eth=1&&netstat -ie|awk -F'[: ]' 'begin{found=0;} { if (match($0,"eth'"$eth"'")) found=1; else if ((1==found) && match($0,"eth")) found=0; if ((1==found) && match($0,"inet addr:") && match($0,"Bcast:")) print $13; }'

uid=`id -u $cur_user`
process_name=$(basename `echo "$process_cmdline"|cut -d" " -f1`)

# 检查被监控的进程是否存在，如果不存在则重启
process_count=`ps -C $process_name h -o euser,args| awk 'BEGIN { num=0; } { if (($1==uid || $1==cur_user) && match($0, process_cmdline)) {++num;}} END { printf("%d",num); }' uid=$uid cur_user=$cur_user process_cmdline="$process_cmdline"`
if test $process_count -lt 1; then
    # 执行重启脚本，要求这个脚本能够将指定的进程拉起来
    printf "\033[0;32;34mrestart \"$process_cmdline\"\033[m\n"
    sh -c "$restart_script" # 注意一定要以“sh -c”方式执行
fi
