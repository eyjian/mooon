#!/bin/sh
# 每次从SVN取下来时，需要运行该脚本，之后再未从SVN取出文件之前不需要运行它
# author: eyjian@qq.com or eyjian@gmail.com
# http://code.google.com/p/mooon
# 2012/12/6晚，参照boost，将first_once.sh改名成bootstrap.sh
#
# 2013/12/25加入前置和后置脚本功能
#

# basedir为源代码存放根目录
basedir=`pwd`
cd $basedir


############################
# 删除所有的.svn目录和文件
############################
#find $basedir -name .svn | xargs rm -fr
#find $basedir/../include -name .svn | xargs rm -fr

# 生成Makefile.am之前执行的脚本，
# before_makefile_am.sh相当于hook，增强bootstrap.sh扩展性
if test -f before_makefile_am.sh; then
    sh before_makefile_am.sh
fi

############################
# 删除字符串前后空格函数
############################
trim()
{
	trimmed=$1
	trimmed=${trimmed%% }
	trimmed=${trimmed## }

	echo $trimmed
}

############################
# 下面为生成Makefile.am文件
############################

# Makefile.am.in模板文件中的_SOURCES部分支持多种写法：
# 1.放置一个可执行脚本文件，由该脚本文件决定使用哪些源代码文件
#   test_SOURCES = x.sh
# 2.放置一段可执行脚本（非文件），由该段脚本决定使用哪些源代码文件
#   test_SOURCES = ls *.cpp
# 3.直接书写源代码文件列表
#   test_SOURCES = a.cpp b.cpp
# 4.保留为空，使用默认的动作（find . -maxdepth 2 | awk '/.cpp$|.cc$|.h$|.hpp$/{printf("%s ", $0)}'）
#   test_SOURCES =
gen_makefile_am()
{
    local line=
    local src_file=$1
    local dest_file=$2

    # do nothing if src not exist
    if test ! -f $src_file; then
        return
    fi

    # remove destination first
    if test -f $dest_file; then
        rm -f $dest_file
    fi

    OLD_IFS=IFS
    IFS=^ # 这里可能会生成问题，需要一个合适的分隔符，一般Makefile.am.in中不会用到^，如果是\n或\r很容易遇到问题
    while read line
    do
        # trim CF/LR
        # 注意“=”两边不能有空格
        line=`echo $line | tr -d "\r\n"`

        # example: libsys_so_SOURCES = x.sh
        if [[ $line =~ .+_SOURCES( ).*=.* ]]; then
            local sources=
            local title=${line%%=*} # libsys_so_SOURCES
            local script=${line#*=} # x.sh

            # trim spaces
            title=$(trim $title)
            script=$(trim $script)

            if test -x "$script"; then # is a executable script file
                sources=`sh $script`
            elif test ! -z "$script"; then # is an executable script, but not file
                sources=`eval "$script" 2>/dev/null`

                # script is not script, but is sources
                if test $? -ne 0; then
                    sources=$script
                fi
            else # empty to use default
                sources=`find . -maxdepth 3 | awk '/.cpp$|.cc$|.h$|.hpp$/{printf("%s ", $0)}'`
            fi

            echo "$title = $sources" >> $dest_file
        else
            # do nothing
            echo "$line" >> $dest_file
        fi
    done < $src_file
    
    IFS=$OLD_IFS
}

rec_subdir()
{
    if test $# -ne 1; then
        echo "Parameter error in rec_subdir"
        exit
    fi

    subdirs=`find $1 -type d`
	echo "generating Makefile.am from Makefile.am.in ..."

    for sub in $subdirs
    do
        # Skip the parent directory
        if test $sub = ".."; then
            continue;
        fi

        if test -f $sub/Makefile.am.in; then
            cd $sub
            gen_makefile_am Makefile.am.in Makefile.am
			echo "generated $sub/Makefile.am"
            cd - > /dev/null
        fi
    done
}

rec_subdir $basedir



############################
# 下面为生成configure.ac文件
############################

# 填写configure.ac中的autoconf版本号
replace_autoconf_version()
{
    autoconf_version=`autoconf --version|head -n1|cut -d' ' -f4`
    sed 's/AUTOCONF_VERSION/'$autoconf_version'/' configure.ac.in > configure.ac
}

# 处理Make.rules文件
check_make_rules()
{
    bit=`getconf LONG_BIT`
    if test $bit -eq 64; then
        sed 's/^MY_CXXFLAGS/#MY_CXXFLAGS/' Make.rules.in > Make.rules
    else
        cp Make.rules.in Make.rules
    fi
}

# 将文件格式从DOS转换成UNIX
d2x()
{
    for file in $*
    do
        src_file=$file
        tmp_file=$file.tmp

        if test -d $src_file; then
            continue
        fi

            tr -d "\r" < $src_file > $tmp_file
            if test $? -eq 0; then
                    mv $tmp_file $src_file
                    #echo "Convert $src_file from the format of DOS to UNIX OK."
            fi
    done
}

# 需要DOS格式转换的文件
#d2x ltmain.sh # 用来生成libtool文件，而它本身则由libtoolize生成
d2x configure.ac.in
d2x Makefile.am
d2x Make.rules.in

check_make_rules
replace_autoconf_version


##########################################
# 下面为生成configure文件和Makefile.in文件
##########################################

aclocal
if test $? -ne 0; then
    echo "aclocal ERROR"
    exit
fi
autoconf
if test $? -ne 0; then
    echo "autoconf ERROR"
    exit
fi
autoheader
if test $? -ne 0; then
    echo "autoheader ERROR"
    exit
fi
libtoolize -f # 用于生成ltmain.sh文件，但有些版本并未见产生ltmain.sh文件
if test $? -ne 0; then
    echo "libtoolize -f ERROR"
    exit
fi
automake -a
if test $? -ne 0; then
    echo "automake -a ERROR"
    exit
fi

# 在一些较高版本，ltmain.sh等不再是在本目录下生成，而是指向了系统的同文件，如：
# ltmain.sh -> /usr/share/libtool/ltmain.sh
# missing -> /usr/share/automake-1.9/missing
# config.guess -> /usr/share/libtool/config.guess
# 所以再执行“chmod +x *.sh”可能会遇到权限不足问题
#chmod +x *.sh
chmod +x configure


#################################################
# 接下来就可以开始执行configure生成Makefile文件了
#################################################
