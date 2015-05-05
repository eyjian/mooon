EXTRA_DIST = Makefile.am

SUBDIRS = plugin_tinyxml

# 检查是否安装了MySQL
#if MYSQL_INSTALLED_USR
#SUBDIRS += plugin_mysql
#else
#if MYSQL_INSTALLED_HOME
#SUBDIRS += plugin_mysql
#else
#if MYSQL_INSTALLED_ENV
#SUBDIRS += plugin_mysql
#endif
#endif
#endif

install-data-hook:
	mkdir -p $(prefix)/include/mooon/plugin&&cp $(top_srcdir)/../include/mooon/plugin/plugin.h $(prefix)/include/mooon/plugin/

uninstall-hook:
	rm -fr $(prefix)/include/mooon/plugin
