#
# 默认认为mooon安装在${HOME}/mooon目录下，可根据实际进行修改
# 编译成功后，生成的可执行程序名为echod，可带一个端口参数，
# 也可不带任何参数运行，默认端口号为2012
#
MOOON=${HOME}/mooon
MOOON_LIB=$(MOOON)/lib/libserver.a $(MOOON)/lib/libmcommon.a
MOOON_INCLUDE=-I$(MOOON)/include

echod: *.cpp
	g++ -g -o $@ *.cpp -lrt -pthread $(MOOON_INCLUDE) $(MOOON_LIB)
    #g++ -g -o $@ *.cpp -pthread $(MOOON_INCLUDE) $(MOOON_LIB) /usr/lib/x86_64-linux-gnu/librt.a

clean:
	rm -f *.o
	rm -f echod
