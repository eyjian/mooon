CPP_FILES=$(shell ls *.cpp)
unit_test:
	CPPFLAGS="-pthread"; \
	LDFLAGS="-lrt -pthread"; \
	bits=`getconf LONG_BIT`; \
	if test $$bits = 32; then \
		CPPFLAGS+=" -march=pentium4"; \
	fi; \
	MYSQL_HOME=abc; \
	if test /usr/local/thirdparty/mysql/lib; then \
		MYSQL_HOME=/usr/local/thirdparty/mysql; \
	else \
		if test ${HOME}/mysql/lib; then \
			MYSQL_HOME=${HOME}/mysql; \
		else \
			if test /usr/local/mysql/lib; then \
				MYSQL_HOME=/usr/local/mysql; \
			fi; \
		fi; \
	fi; \
	if test $$MYSQL_HOME = "abc"; then \
		echo "error: not found MySQL at:"; \
		echo "1) /usr/local/thirdparty/mysql"; \
		echo "2) ${HOME}/mysql"; \
		echo "3) /usr/local/mysql"; \
		exit; \
	fi; \
	dos2unix run.sh;chmod +x run.sh; \
	for cpp_file in $(CPP_FILES); \
	do \
		name=`basename $$cpp_file .cpp`; \
		g++ -g -c -I../../include $$cpp_file $$CPPFLAGS; \
		g++ -g -o $$name $$name.o -march=pentium4 $$CPPFLAGS $$LDFLAGS ../../src/common/libmooon.a $$MYSQL_HOME/lib/libmysqlclient_r.a; \
	done

clean:
	rm -f *.o
