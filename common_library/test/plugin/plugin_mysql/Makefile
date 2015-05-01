CPP_FILES=$(shell ls *.cpp)
unit_test:
	dos2unix run.sh;chmod +x run.sh;
	for cpp_file in $(CPP_FILES); \
	do \
		name=`basename $$cpp_file .cpp`; \
		g++ -g -o $$name -I../../../include ../../../src/plugin/plugin_mysql/libmooon_mysql.a ../../../src/sys/libmooon_sys.a ../../../src/utils/libmooon_utils.a $$cpp_file; \
	done
