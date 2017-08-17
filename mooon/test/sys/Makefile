include ../../src/Make.rules

CPP_FILES=$(shell ls *.cpp)
unit_test:
	CPPFLAGS="-pthread"; \
	CPPFLAGS+=" $(MOOON_HAVE_MYSQL) $(MOOON_HAVE_SQLITE3) $(MOOON_HAVE_CURL)"; \
	CPPFLAGS+=" $(MYSQL_INCLUDE) $(SQLITE3_INCLUDE) $(CURL_INCLUDE)"; \
	LDFLAGS="-lz -lrt -pthread"; \
	LDFLAGS+=" $(MYSQL_LIB) $(SQLITE3_LIB) $(CURL_LIB)"; \
	bits=`getconf LONG_BIT`; \
	if test $$bits = 32; then \
		CPPFLAGS+=" -march=pentium4"; \
	fi; \
	dos2unix run.sh;chmod +x run.sh; \
	for cpp_file in $(CPP_FILES); \
	do \
		name=`basename $$cpp_file .cpp`; \
		g++ -g -c -I../../include $$cpp_file $$CPPFLAGS; \
		g++ -g -o $$name $$name.o ../../src/common/libmooon.a $$CPPFLAGS $$LDFLAGS; \
	done

clean:
	rm -f *.o

