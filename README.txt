推荐使用cmake编译

cmake安装方法：
cmake -DCMAKE_INSTALL_PREFIX=<installation directory> .

“-DCMAKE_INSTALL_PREFIX=”后跟安装目录，如/usr/local/thirdparty/mooon：
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/thirdparty/mooon .
如果以debug方式编译，指定参数：-DCMAKE_BUILD_TYPE=Debug
如果以Release方式编译，指定参数：-DCMAKE_BUILD_TYPE=Release

子目录说明：
mooon MOOON的基础类库，封装了字符串操作、常见的系统调用如锁、以及网络相关的，编译后生成库文件libmooon.a
application 一些应用的实现，如唯一ID服务，依赖mooon
component 公共组件的实现，依赖mooon
