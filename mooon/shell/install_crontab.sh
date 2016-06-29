#!/bin/sh
# writed by yijian on 2013/1/19
# generic script used to install an iterm into crontab
# http://code.google.com/p/mooon/source/browse/trunk/common_library/shell/install_crontab.sh

# $monitor_name used to monitor the process named by $process_name,
# if $process_name is hangup, $monitor_name will reboot it by $start_name.
# before using $monitor_name, $process_name should be running,
# the installation will get the path information automatically.
# **PLEASE NOTE**: all files must be placed in the same directory.
process_name=$1    # the fullname of process monitored (but not included directory)
monitor_name=$2    # the path of monitor script (maybe included directory)
start_name=$3      # the path of start script (maybe included directory)
timedate_fields=$4 # five time and date fields
me=`whoami`

# check cmdline
if test $# -ne 4; then
	echo "usage: process_name monitor_name start_name time_date"
	echo "EXAMPLE1: $0 mooon process_monitor.sh start_test.sh \"* * * * *\""
	echo "EXAMPLE2: $0 mooon /tmp/process_monitor.sh /tmp/start_test.sh \"* * * * *\""
	echo "monitor_name or start_name must be in the same directory with process_name, "
	echo "if monitor_name or start_name don't include directory"
	exit 1
fi

# get pid & username
eval $(ps -C $process_name h -o pid,euser | awk -F" " '{ printf("pid=%s\nuser=%s\n",$1,$2); }')
if test $? -ne 0; then
	echo "$process_name NOT RUNNING now"
	exit 1
fi

if test "X$me" != "X$user"; then
	echo "$process_name NOT RUNNING now"
	exit 1
fi

# get file path
filepath=`readlink /proc/$pid/exe`
if test $? -ne 0; then
	exit 1
fi

# get directory path
binpath=`dirname $filepath`

# check files
if test -x $monitor_name; then
	monitor_path=$monitor_name
else
	monitor_path=$binpath/$monitor_name
fi
if test -x $start_name; then
	start_path=$start_name
else
	start_path=$binpath/$start_name
fi

if test ! -x $monitor_path; then
	echo "$monitor_path not exist"
	exit 1
fi
if test ! -x $start_path; then
	echo "$start_path not exist"
	exit 1
fi

# install crontab if not exist
crontab -l | grep "$start_path" > /dev/null 2>&1
if test $? -ne 0; then
	crontab -l > crontab.tmp
	echo -e "\n# the following monitor added by `basename $0`($process_name) on `date +"%Y-%m-%d %H:%M:%S"`" >> crontab.tmp
	echo "$timedate_fields $monitor_path $process_name $start_path" >> crontab.tmp
	crontab crontab.tmp
	if test $? -ne 0; then
		exit 1
	fi

	rm crontab.tmp
fi

exit 0
