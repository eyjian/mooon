#!/bin/sh
# writed by yijian on 2013/1/19
# generic script used to uninstall an item from crontab
# http://code.google.com/p/mooon/source/browse/trunk/common_library/shell/install_crontab.sh

# key is the item need to remove from crontab
key=$1

crontab -l | grep -v "$key" > crontab.tmp
crontab crontab.tmp

exit 0
