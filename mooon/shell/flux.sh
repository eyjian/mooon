#!/bin/sh
# Writed by yijian on 2008-3-20
# 流量统计工具
# 可带一个两个：
# 参数1：网卡名，如eth0或eth1等
# 参数2：统计次数
# 输出格式：统计时间,入流量(Kbps),入流量(Mbps),出流量(Kbps),出流量(Mbps)

# Please edit the followings
EthXname=eth1 # Interface name
StatFreq=2 # Seconds
StatTimes=0 # 统计几次后退出，0表示永远循环
i=0

if test $# -ge 1; then
	EthXname=$1
fi
echo "Destination: $EthXname"

if test $# -ge 2; then
	StatTimes=$2
fi

# Don't change
influx_kbps=0
outflux_kbps=0
influx_mbps=0
outflux_mbps=0
unsigned_long_max=4294967295

# 检查是否存在EthXname
Ethname=`cat /proc/net/dev|grep $EthXname|awk -F"[: ]+" '{ printf("%s", $2); }'`
if test "$EthXname" != "$Ethname"; then
	echo "Please set EthXname first before running"
	echo "Usage: flux.sh ethX times"
	echo "Example: flux.sh eth1 2"
	exit 1
fi
# 进一步检查是否存在EthXname
netstat -ie|grep $EthXname> /dev/null 2>&1 
if test $? -ne 0; then
	echo "Please set EthXname first before running"
	echo "Usage: flux.sh ethX"
	echo "Example: flux.sh eth0"
	exit 1
fi

# 初始化
influx1_byte=`cat /proc/net/dev|grep $EthXname|awk -F"[: ]+" '{ printf("%d", $3); }'`
outflux1_byte=`cat /proc/net/dev|grep $EthXname|awk -F"[: ]+" '{ printf("%d", $11); }'`

printf "\033[1;33mDate,IN-Kbps,IN-Mbps,OUT-Kbps,OUT-Mbps\033[m\n"
while test 2 -gt 1;
do
	sleep $StatFreq
	#influx2_byte=`cat /proc/net/dev|grep $EthXname|awk -F"[: ]+" '{ printf("%d", $3); }'`
	#outflux2_byte=`cat /proc/net/dev|grep $EthXname|awk -F"[: ]+" '{ printf("%d", $11); }'`
	inout_bytes=`awk -F"[: ]+" /$EthXname/'{ printf("%s %s", $3, $11) }' /proc/net/dev`
	inout_bytes_array=($inout_bytes)
	influx2_byte=${inout_bytes_array[0]}
	outflux2_byte=${inout_bytes_array[1]}

	dd=`date +'%Y-%m-%d/%H:%M:%S'`
	if test $influx2_byte -ge $influx1_byte; then
		let influx_byte=$influx2_byte-$influx1_byte
	else
		let influx_byte=$unsigned_long_max-$influx1_byte
		let influx_byte=$influx_byte+$influx2_byte
	fi
	if test $outflux2_byte -ge $outflux1_byte; then
		let outflux_byte=$outflux2_byte-$outflux1_byte
	else
		let outflux_byte=$unsigned_long_max-$outflux1_byte
		let outflux_byte=$outflux_byte+$outflux2_byte
	fi
		
	let influx_byte=$influx_byte/$StatFreq
	let outflux_byte=$outflux_byte/$StatFreq
	# TO bps
	let influx_bps=$influx_byte*8
	let outflux_bps=$outflux_byte*8
	# To kbps
	let influx_kbps=$influx_bps/1024
	let outflux_kbps=$outflux_bps/1024
	# To mbps
	let influx_mbps=$influx_kbps/1024
	let outflux_mbps=$outflux_kbps/1024
	# SHOW on screen

	# COLUMN: Date,IN-Kbps,IN-Mbps,OUT-Kbps,OUT-Mbps
	#echo "$dd,${influx_kbps}Kbps,${influx_mbps}Mbps,${outflux_kbps}Kbps,${outflux_mbps}Mbps"
	printf "$dd,${influx_kbps}Kbps,"
	printf "\033[1;33m${influx_mbps}Mbps\033[m,"
	printf "${outflux_kbps}Kbps,"
	printf "\033[1;33m${outflux_mbps}Mbps\033[m\n"
	
	let influx1_byte=influx2_byte
	let outflux1_byte=outflux2_byte
    
    # 执行指定次数后退出
    if test $StatTimes -gt 0; then
        i=$(($i+1))
        if test $i -ge $StatTimes; then
            break
        fi
    fi
done
