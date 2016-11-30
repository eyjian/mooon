#!/bin/sh
# writed by yijian on 2016/7/11
# filename mem.sh
# 用于统计指定进程的虚拟内存和物理内存占用，以分析是否有内存泄漏

if test $# -lt 1; then
    echo "uage: mem.sh pid interval(seconds)"
    exit 1
fi
if test $# -eq 2; then
	interval=$2
else
	interval=60
fi
if test $interval -lt 2; then
	interval=2
fi

pid=$1
file=$pid.mem
rm -f $file

while true
do
    filesize=$(ls -l /tmp/$pid.mem 2>/dev/null|cut -d' ' -f5)
	if test ! -z $filesize; then
		if test $filesize -gt 1048576; then
       		mv /tmp/$file /tmp/$file.old
		fi
    fi
	virt=0
	res=0
    eval $(cat /proc/$pid/statm 2>/dev/null| awk '{ printf("virt=%d\nres=%d", $1*4096/1024/1024,$2*4096/1024/1024); }')
	if test $virt -eq 0 -a $res -eq 0; then
		break
	fi
    echo "[`date '+%Y-%m-%d %H:%M:%S'`] ${virt}m ${res}m" | tee -a /tmp/$file
    sleep $interval
done

