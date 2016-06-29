#
# 默认认为mooon安装在${HOME}/mooon目录下，可根据实际进行修改
# 编译成功后，生成的可执行程序名为web_getter，
# web_getter使用方式，带3个参数，如下：
# --port Web server的服务端口号，这个参数是可选的，默认值为80
# --dn Web server服务的域名
# --url 需要获取的网页
#
MOOON=${HOME}/mooon
MOOON_LIB=$(MOOON)/lib/libhttp_parser.a $(MOOON)/lib/libdispatcher.a $(MOOON)/lib/libmcommon.a
MOOON_INCLUDE=-I$(MOOON)/include

web_getter: *.cpp
	g++ -g -o $@ *.cpp -lrt -pthread $(MOOON_INCLUDE) $(MOOON_LIB)
    #g++ -g -o $@ *.cpp -pthread $(MOOON_INCLUDE) $(MOOON_LIB) /usr/lib/x86_64-linux-gnu/librt.a

clean:
	rm -f *.o
	rm -f web_getter
