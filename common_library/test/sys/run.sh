#!/bin/sh

export LD_LIBRARY_PATH=.:../../src/utils:../../src/net:../../src/sys:$LD_LIBRARY_PATH

FILES=`ls ut_*|grep -v ".cpp"|grep -v ".o"`
for file in $FILES;
do
	sh -c ./$file
done
