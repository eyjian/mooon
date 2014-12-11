#!/bin/sh
# Writed by JianYi On 2008-5-20

# 变量定义
log_filename= # 日志文件名
log_bakeup_nubmer=10 # 日志备份个数
log_rotate_bytes=102400000 # 日志滚动字节数

# 用途：得到文件大小
# 参数：文件名
# 返回值：如果失败返回0，否则返回文件大小
get_file_size()
{
	file_size=`ls --time-style=long-iso -l $1 2>/dev/null|cut -d" " -f5`
	if test -z $size; then
		echo "0"
	else
		echo $file_size
	fi
}

# 用途：得到日志文件索引
# 参数：日志文件名的基名，即不包括索引部分的名字，如：test.log，而不是test.log.1
# 返回值：成功返回索引值，失败返回-1
get_log_file_index()
{
    log_file_basename=$1
    index=`ls $log_file_basename.[0-9] 2>/dev/null|cut -d"." -f3|sort -n|sed -n '$p'`
    if test -z $index; then
        echo "-1"
    else
        echo $index
    fi
}

# 用途：滚动日志文件
# 参数：日志文件名
rotate_log_file()
{
	log_file_basename=$1
	index=`get_log_file_index $log_file_basename`
	if test $index -eq -1; then
		mv $log_file_basename $log_file_basename.0
		return
	elif test $index -ge $log_file_roll_num; then
        index=$log_file_roll_num
	fi
	
	# 循环滚动文件, 索引号越大文件内容越旧
    while true
    do
        let index_next=$index+1
        if test -f $log_filename.$index; then
            mv $log_file_basename.$index $log_file_basename.$index_next
        fi
        
        if test $index -eq 0; then
            # 结束循环
            if test -f $log_file_basename; then
                mv $log_file_basename $log_file_basename.0
            fi
            return
        else
            # 计数器增一
            let index=$index-1
        fi                
    done
}

# 用途：写日志函数
# 参数：日志内容字符串
log_write()
{
	# 得到日志文件大小
	log_size=`get_file_size $log_filename`
	
	# 如果大于滚动大小
	if test $log_size -gt $log_roll_bytes; then
		roll_log_file $log_filename
	else
		# 得到当前时间
		now="`date +'%Y-%m-%d/%H:%M:%S'`"
		echo "[$now]$1" >> $log_filename
	fi
}
