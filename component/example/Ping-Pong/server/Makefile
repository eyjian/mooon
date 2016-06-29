#
# 默认认为mooon安装在${MOOON_INSTALL}目录下，可根据实际进行修改
# 编译成功后，生成的可执行程序名为pp_server，
# ./pp_server --ip=127.0.0.1 --port=1234
#
MOOON=${MOOON_INSTALL}
MOOON_LIB=$(MOOON)/lib/libserver.a $(MOOON)/lib/libmcommon.a
MOOON_INCLUDE=-I../common -I$(MOOON)/include

pp_server: *.cpp
	g++ -g -o $@ *.cpp -lrt -pthread $(MOOON_INCLUDE) $(MOOON_LIB)
    #g++ -g -o $@ *.cpp -pthread $(MOOON_INCLUDE) $(MOOON_LIB) /usr/lib/x86_64-linux-gnu/librt.a

clean:
	rm -f *.o
	rm -f pp_server
