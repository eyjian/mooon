#
# 默认认为mooon安装在${MOOON_INSTALL}目录下，可根据实际进行修改
# 编译成功后，生成的可执行程序名为pp_client，
# pp_client使用方式，带3个参数，如下：
# --ping pong server的服务端口号
# --ping pong server服务的ip，这个参数是可选的，默认值为127.0.0.1
# ./pp_client --ip=127.0.0.1 --port=1234 --thread_count=1 --sender_count=1 
#

MOOON=${MOOON_INSTALL}
MOOON_LIB=$(MOOON)/lib/libhttp_parser.a $(MOOON)/lib/libdispatcher.a $(MOOON)/lib/libmcommon.a
MOOON_INCLUDE=-I../common -I$(MOOON)/include

pp_client: *.cpp
	g++ -g -o $@ *.cpp -lrt -pthread $(MOOON_INCLUDE) $(MOOON_LIB)

clean:
	rm -f *.o
	rm -f pp_client
