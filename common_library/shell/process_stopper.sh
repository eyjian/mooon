#!/bin/sh
# http://code.google.com/p/mooon
# Created by yijian on 2012/7/23
# 通用的停止指定名的进程
# 特色：
# 1. 只会停止当前用户名下的进程
# 2. 可以指定命令行参数，可执行精准停止

# 检查参数
# 参数1：需要停止的进程名或它的完整命令行或部分命令行
if test $# -ne 1; then
	printf "\033[1;33musage: $0 process_cmdline\033[m\n"
	exit 1
fi

process_cmdline=$1    # 进程名或进程运行命令行
cur_user=`whoami`     # 当前用户
uid=`id -u $cur_user` # 用户ID

# 取得进程名
process_name=$(basename `echo "$process_cmdline"|cut -d" " -f1`)
# 得到进程的ID列表
pid_set=`ps -C "$process_name" h -o euser,pid,args|awk '{ if (($1==uid || $1==cur_user) && match($0, process_cmdline)) printf("%s\n", $2); }' uid=$uid cur_user=$cur_user process_cmdline="$process_cmdline"`

# 循环kill掉所有的进程
for pid in $pid_set
do
	#echo $pid;exit; # 测试用
	kill $pid     # 先发SIGTERM信号
	sleep 2       # 过2秒再发SIGKILL信号
	kill -9 $pid
done
