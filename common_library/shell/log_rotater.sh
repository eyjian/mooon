#!/bin/sh
# http://code.google.com/p/mooon
# Writed by yijian on 2012/7/22
# 本脚本功能：
# 1) 实现一个通用的按大小的日志滚动
# 2) 要求日志文件扩展名为“.log”，否则请稍修改代码
# 3) 支持处理多个目录下的日志文件，如果需要使用这个功能，
#    必须启用dirs_list参数
#
# 为何要写一个这样的东东？
# 答：因为在日常中，经常发现程序输出重定向，
#     或脚本的输出，如果不处理的话，就可能导致
#     单个文件过大，甚至爆满整个磁盘；而每套脚本
#     都重复写一个日志滚动逻辑，显然是件无聊的事,
#     甚至一些人懒得做处理，日子一久，就会发现一个超大的
#     日志文件，本脚本希望可以帮助解决这个问题，让大家无
#     后顾之忧。
#
# 使用方法：
# 1) 把脚本复制到日志文件所在目录，
#    然后以nohup ./log_rotater.sh > /dev/null & 等方式启动脚本即可
# 2) 除了上面的方法，也可以将log_rotater.sh任何目录下，
#    但这个时候，应当将backup_dir的值修改为日志文件所在目录
#
# sudo dpkg-reconfigure dash
# 新版本Ubuntu默认使用dash作为shell，
# 这个shell功能较弱，不支持数组等，但速度快，
# 可采取如下办法检测是何种shell：
# ls -l `which sh`

# 可根据需要修改以下参数
backup_count=10 # 日志滚动的个数
backup_size=$((1024 * 1024 * 200)) # 单个日志文件大小
backup_interval=60 # 检测的间隔时间，单位为秒

# 如果dirs_list指定的文件存在，则从dirs_list中读取目录，
# 否则仅处理backup_dir指定的单个目录
# 往dirs_list指定文件增减目录时，不需要重启log_rotater.sh
backup_dir=. # 日志文件所在目录
dirs_list=./dirs.list # 存储目录列表的文件，要求一行一个目录

# 处理单个目录下的日志滚动
scan_single_dir()
{
	dir=$1
	cd $dir
	if test $? -ne 0; then
		return
	fi

	# 用到了awk给外部变量赋值的特性
	eval $(ls -l --time-style=long-iso *.log 2>/dev/null|awk '{ printf("filesize=%s\nfiledate=%s\nfilename=%s\n", $5,$6,$8); }')
	if test $? -ne 0; then
		return
	fi
	if test -z $filename; then
		return
	fi

	if test $filesize -gt $backup_size; then
		file_index=$(($backup_count - 1))
		while test $file_index -gt 1; do
			new_filename="${filename}.$file_index"
			old_filename="${filename}.$(($file_index - 1))"
			if test -f "$old_filename"; then
				mv "$old_filename" "$new_filename"
			fi
	
			file_index=$(($file_index - 1))
		done
	
    	# 这里需要使用truncate，而不能使用mv，
        # 因为需要保持文件的inode不变
		cp "$filename" "${filename}.1"
		truncate -s 1024 "$filename"
	fi
	
	cd -
	sleep 1
}

# 循环检测
while true; do
	if test ! -f $dirs_list; then
		scan_single_dir $backup_dir
	else
		while read dirpath
		do
			scan_single_dir $dirpath
		done < $dirs_list
	fi

	sleep $backup_interval
done
