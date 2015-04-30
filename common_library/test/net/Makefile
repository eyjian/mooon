CPP_FILES=$(shell ls *.cpp)
unit_test:
	dos2unix run.sh;chmod +x run.sh;
	for cpp_file in $(CPP_FILES); \
	do \
		name=`basename $$cpp_file .cpp`; \
		g++ -g -o $$name -I../../include -L../../src/utils -lutils -L../../src/sys -lsys -L../../src/net -lnet $$cpp_file; \
	done
