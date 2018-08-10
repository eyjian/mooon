#!/bin/sh
# https://github.com/eyjian/mooon
# Created by yijian on 2018/8/10
# 根据IP设置机器名工具，
# 结合mooon_ssh和mooon_upload两个工具，可实现批量操作
#
# 参数：
# IP和机器名映射关系配置文件
#
# IP和机器名映射关系配置文件格式：
# IP+分隔符+机器名
# 示例：
# 192.168.31.32 zhangsan-32
# 分隔符可为空格、TAB、逗号、分号和竖线这几种
#
# 注意：hadoop要求hostname不能带下划线
#
# 使用方法：
# 1）利用mooon_upload，批量将本脚本文件发布到所有目标机器（需要修改hostname的机器）上
# 2）利用mooon_upload，批量将映射关系配置文件发布到所有目标机器上
# 3）利用mooon_ssh，批量执行本脚本完成主机名设置

# 本机IP所在网卡
ethX=eth1

# 参数检查
if test $# -ne 1; then
    echo -e "\033[1;33musage\033[m:\nset_hostname.sh conffile"
    echo ""
    echo "conffile format:"
    echo "IP + separator + hostname"
    echo ""
    echo "example:"
    echo "192.168.31.1 zhangsan-1"
    echo "192.168.31.2 zhangsan-2"
    echo "192.168.31.3 zhangsan-3"
    echo ""
    echo "available separators: space, TAB, semicolon, comma, vertical line"
    exit 1
fi

# 检查配置文件是否存在和可读
conffile=$1
if test ! -f $conffile; then
    echo -e "file \`$conffile\` \033[0;32;31mnot exists\033[m"
    exit 1
fi
if test ! -r $conffile; then
    echo -e "file \`$conffile\` \033[0;32;31mno permission to read\033[m"
    exit 1
fi

# 取本机IP需要用到netstat
which netstat > /dev/null 2>&1
if test $? -ne 0; then
    echo "\`netstat\` command \033[0;32;31mnot available\033[m"
    exit 1
fi

# 取得hostname的配置文件
# 注意不同Linux发行版本的hostname文件可能不同
hostname_file=/etc/hostname
if test ! -w $hostname_file; then
    echo "can not write $hostname_file, or $hostname_file not exits"
    exit 1
fi

# 取本地IP
local_ip=`netstat -ie|awk -F'[ :]+' -v ethX=$ethX 'BEGIN{ok=0;} {if (match($0, ethX)) ok=1; if ((1==ok) && match($0,"inet")) { ok=0; if (7==NF) printf("%s\n",$3); else printf("%s\n",$4); } }'`
if test -z "$local_ip"; then
    echo "can not get local IP of \`$ethX\`"
    exit 1
fi

# 读取文件，找到本机待设置的新主机名
while read line
do
    if test -z "$line"; then
        break
    fi

    eval $(echo $line |awk -F'[ \t|,;]+' '{ printf("ip=%s\nhostname=%s\n", $1, $2); }')
    #echo "[IP] => $ip  [HOSTNAME] => $hostname"    
    if test "$ip" = "$local_ip"; then
        echo "$ip => $hostname"
        echo "$hostname" > $hostname_file
        exit 0
    fi
done < $conffile

echo "no item for $local_ip"
exit 1
