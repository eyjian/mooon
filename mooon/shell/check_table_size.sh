#!/bin/sh
# writed by yijian on 2015/9/10
# 批量检查表大小（记录数）脚本
# 可以针对单个库的所有表，也可以针对所有库的所有表

db_host=127.0.0.1
db_port=3306
db_user=root
db_password=
db_name=

if test "$1" = "--help"; then
	# 用法1：指定所有参数，依次为DB的IP地址、访问DB的用户名、访问DB的密码、DB的端口号和库名称
	echo "usage1: check_table_size.sh db_host db_user db_password db_port db_name"
	# 用法2：不指定用户名、密码、端口和库名参数，默认用户名为root，默认密码为空，默认端口号为3306，默认检查所有库
	echo "usage2: check_table_size.sh db_host, default [root: root, passowrd empty, port: 3306, all databases]"
	# 用法3：不指定密码、端口和库名参数，默认密码为空，默认端口号为3306，默认检查所有库
	echo "usage3: check_table_size.sh db_host db_user, default [passowrd empty, port: 3306, all databases]"
	# 用法4：不指定端口和库名参数，默认端口号为3306，默认检查所有库
	echo "usage4: check_table_size.sh db_host db_user db_password, default [port: 3306, all databases]"
	# 使用示例：
	echo "example: ./check_table_size.sh 127.0.0.1 root 'test!~@' 3306"
	exit 1
fi

# 如果命令行参数指定了
if test $# -gt 0; then
	db_host=$1
fi
if test $# -gt 1; then
	db_user=$2
fi
if test $# -gt 2; then
	db_password=$3
fi
if test $# -gt 3; then
	db_port=$4
fi
if test $# -gt 4; then
	db_name=$5
fi

# 如果没有指定库名，则检查所有库
databases=$db_name
if test -z $db_name; then
	echo "mysql -h$db_host -P$db_port -u$db_user -p$db_password"
	databases=`mysql -h$db_host -P$db_port -u$db_user -p$db_password -e"show databases"`
	if test $? -ne 0; then
		exit 1
	fi
fi

# 存放结果的文件
table_size_file=./.table_size.txt
if test $table_size_file; then
	rm -f $table_size_file
fi

for database in $databases
do
	if test $database != "Database"; then
		tables=`mysql -h$db_host -P$db_port -u$db_user -p$db_password $database -e"show tables"`
		if test $? -eq 0; then
			for table in $tables
			do
				if test $table != "Tables_in_$database"; then
					echo "checking table[$database.$table] ..."
					table_size=`mysql -h$db_host -P$db_port -u$db_user -p$db_password $database -e"SELECT count(1) FROM $table"`
					table_size=`echo $table_size |cut -d' ' -f2`
					echo "$table_size $database.$table"  >> $table_size_file
					echo "table[$database.$table] size is $table_size"
				fi
			done
		fi
	fi
done

if test -f $table_size_file; then
	sort -r -n $table_size_file
	#rm -f $table_size_file
fi
