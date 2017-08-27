#!/bin/sh
# https://github.com/eyjian/mooon/blob/master/common_library/shell/process_monitor.sh
# Created by yijian on 2012/7/23
#
# 运行日志：/tmp/process_monitor.log，由于多进程同时写，不一定完整，仅供参考。
# 请放到crontab中运行，如（注意要以后台方式运行，因为脚本是常驻不退出的）：
# * * * * * /usr/local/bin/process_monitor.sh /usr/sbin/rinetd /usr/sbin/rinetd > /dev/null 2>&1 &
#
# 进程监控脚本，当指定进程不存在时，执行重启脚本将它拉起
# 特色：
# 1.本监控脚本可重复执行，它会自动做互斥
# 2.互斥不仅依据监控脚本文件名，而且包含了它的命令行参数，只有整体相同时互斥才生效
# 3.对于被监控的进程，可以只指定进程名，也可以包含命令行参数
# 4.不管是监控脚本还是被监控进程，总是只针对属于当前用户下的进程
#
# 如果本脚本手工运行正常，但在crontab中运行不正常，则可考虑检查下ps等命令是否可在crontab中正常运行
#
# 如何监控脚本？
# 不同环境可能存在差异，需要具体测试，下面是一个示例，但不同环境可能存在差异。
# 假设有一需要监控脚本：/home/zhangsan/bin/mem.sh
# 则crontab可配置为：
# * * * * * /usr/local/bin/process_monitor.sh "mem.sh" "/home/zhangsan/bin/mem.sh &"
#
# 下面带路径的用法可能导致监控失效，mem.sh被反复大量拉起，
# 而监控对象非脚本时，则需要带上路径，这是监控脚本和程序的不同之处：
# * * * * * /usr/local/bin/process_monitor.sh "/home/zhangsan/bin/mem.sh" "/home/zhangsan/bin/mem.sh &"
#
# 当然如果不以“&”的后台方式运行也是可以的，但不推荐这样，因为这阻塞了process_monitor.sh，
# 实际相当于process_monitor.sh直接在运行。
#
# 如果不同路径下，有相同名称的脚本，则需要借助参数进行区分，如：
# * * * * * /usr/local/bin/process_monitor.sh "mem.sh eth1" "/home/wangwu/bin/mem.sh eth1 &"
# * * * * * /usr/local/bin/process_monitor.sh "mem.sh eth0" "/home/zhangsan/bin/mem.sh eth0 &"
#
# 而程序则可直接路径区分，如（mem为一可执行程序，非脚本文件）：
# * * * * * /usr/local/bin/process_monitor.sh "/home/wangwu/bin/mem" "/home/wangwu/bin/mem eth1 &"
# * * * * * /usr/local/bin/process_monitor.sh "/home/zhangsan/bin/mem" "/home/zhangsan/bin/mem eth0 &"

# 实际中，遇到脚本在crontab中运行时，找不到ls和ps等命令
# 原来是有些环境ls和ps位于/usr/bin目录下，而不是常规的/bin目录
export PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
trap "" SIGPIPE # 忽略SIGPIPE

# 需要指定个数的命令行参数
# 参数1：被监控的进程名（可以包含命令行参数）
# 参数2：重启被监控进程的脚本
if test $# -ne 2; then
    printf "\033[1;33musage: $0 process_cmdline restart_script\033[m\n"
    printf "\033[1;33mexample: /usr/local/bin/process_monitor.sh \"/usr/sbin/rinetd\" \"/usr/sbin/rinetd\"\033[m\n"
    printf "\033[1;33mplease install process_monitor.sh into crontab by \"* * * * *\"\033[m\n"
    exit 1
fi

process_cmdline="$1" # 需要监控的进程名，或完整的命令行，也可以为部分命令行
restart_script="$2"  # 用来重启进程的脚本，要求具有可执行权限
monitor_interval=2   # 定时检测时间间隔，单位为秒
start_seconds=5      # 被监控进程启动需要花费多少秒
cur_user=`whoami`    # 执行本监控脚本的用户名

# 取指定网卡上的IP地址
#eth=1&&netstat -ie|awk -F'[: ]' 'begin{found=0;} { if (match($0,"eth'"$eth"'")) found=1; else if ((1==found) && match($0,"eth")) found=0; if ((1==found) && match($0,"inet addr:") && match($0,"Bcast:")) print $13; }'

# 下面这段脚本，用来阻止多个监控脚本进程出现
uid=`id -u $cur_user`
self_name=`basename $0`
self_cmdline="$0 $*"
process_name=$(basename `echo "$process_cmdline"|cut -d" " -f1`)
process_match="${process_cmdline#* }" # 只保留用来匹配的参数部分
process_match=$(echo $process_match) # 去掉前后的空格

# 用来做互斥，
# 以保证只有最先启动的能运行，
# 但若不同参数的彼此不相互影响，
# 这样保证了可同时对不同对象进行监控。
# 因为trap命令对KILL命令无效，所以不能通过创建文件的方式来互斥！
active=0

# 日志文件，可能多个用户都在运行，
# 所以日志文件名需要加上用户名，否则其它用户可能无权限写
log_filepath=/tmp/process_monitor-$cur_user.log
# 日志文件大小（10M）
log_filesize=10485760

# 写日志函数，带1个参数：
# 1) 需要写入的日志
log()
{
    # 创建日志文件，如果不存在的话    
    if test ! -f $log_filepath; then
        touch $log_filepath
    fi

    record=$1
    # 得到日志文件大小
    file_size=`ls --time-style=long-iso -l $log_filepath 2>/dev/null|cut -d" " -f5`

    # 处理日志文件过大
    # 日志加上头[$process_cmdline]，用来区分对不同对象的监控
    if test ! -z "$file_size"; then        
        if test $file_size -lt $log_filesize; then
            printf "[$process_cmdline]$record"
            printf "[$process_cmdline]$record" >> $log_filepath
        else
            printf "[$process_cmdline]$record" >> $log_filepath
            mv $log_filepath $log_filepath.bak # 备份

            printf "[$process_cmdline][`date +'%Y-%m-%d %H:%M:%S'`]truncated\n"
            printf "[$process_cmdline][`date +'%Y-%m-%d %H:%M:%S'`]truncated\n" > $log_filepath

            printf "[$process_cmdline]$record"
            printf "[$process_cmdline]$record" >> $log_filepath            
        fi
    fi
}

# 以死循环方式，定时检测指定的进程是否存在
# 一个重要原因是crontab最高频率为1分钟，不满足秒级的监控要求
while true; do
    self_count=`ps -C $self_name h -o euser,args| awk 'BEGIN { num=0; } { if (($1==uid || $1==cur_user) && match($0, self_cmdline)) {++num;}} END { printf("%d",num); }' uid=$uid cur_user=$cur_user self_cmdline="$self_cmdline"`
    if test ! -z "$self_count"; then 
        if test $self_count -gt 2; then 
            log "\033[0;32;31m[`date +'%Y-%m-%d %H:%M:%S'`]$0 is running[$self_count/active:$active], current user is $cur_user.\033[m\n"
            # 经测试，正常情况下一般为2，
            # 但运行一段时间后，会出现值为3，因此放在crontab中非常必要
            # 如果监控脚本已经运行，则退出不重复运行
            if test $active -eq 0; then
                exit 1
            fi
        fi
    fi

    # 检查被监控的进程是否存在，如果不存在则重启
    if test -z "$process_match"; then
        process_count=`ps -C $process_name h -o euser,args| awk 'BEGIN { num=0; } { if (($1==uid || $1==cur_user)) {++num;}} END { printf("%d",num); }' uid=$uid cur_user=$cur_user`
    else
        process_count=`ps -C $process_name h -o euser,args| awk 'BEGIN { num=0; } { if (($1==uid || $1==cur_user) && match($0, process_match)) {++num;}} END { printf("%d",num); }' uid=$uid cur_user=$cur_user process_match="$process_match"`
    fi
    if test ! -z "$process_count"; then
        if test $process_count -lt 1; then
            # 执行重启脚本，要求这个脚本能够将指定的进程拉起来
            log "\033[0;32;34m[`date +'%Y-%m-%d %H:%M:%S'`]restart \"$process_cmdline\"\033[m\n"
            sh -c "$restart_script" >> $log_filepath 2>&1 # 注意一定要以“sh -c”方式执行
			
			# sleep时间得长一点，原因是启动可能没那么快，以防止启动多个进程
			# 在某些环境遇到sleep无效，正常sleep后“$?”值为0，则异常时变成“141”，
			# 这个是因为收到了信号13，可以使用“trap '' SIGPIPE”忽略SIGPIPE。
			sleep $start_seconds
		else
			sleep $monitor_interval
        fi
	else
		sleep $monitor_interval
    fi

    active=1
done
exit 0
