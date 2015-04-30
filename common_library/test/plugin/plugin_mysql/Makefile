CPP_FILES=$(shell ls *.cpp)
unit_test:
	dos2unix run.sh;chmod +x run.sh;
	for cpp_file in $(CPP_FILES); \
	do \
		name=`basename $$cpp_file .cpp`; \
		g++ -g -o $$name -I../../../include ../../../src/plugin/plugin_mysql/libxmysql.a ../../../src/sys/libsys.a ../../../src/utils/libutils.a $$cpp_file; \
	done
