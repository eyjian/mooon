组件说明：
src/dispatcher 多线程多连接客户端分发组件，为一个可编程库
src/http_parser HTTP包解析库
src/server 服务端框架库
src/agent 代理框架
src/observer 用来数据统计的观察者

cmake安装方法：
cmake -DCMAKE_INSTALL_PREFIX=<installation directory> .

“-DCMAKE_INSTALL_PREFIX=”后跟安装目录，如/usr/local/mooon：
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/mooon .

如果以debug方式编译，指定参数：-DCMAKE_BUILD_TYPE=Debug
如果以Release方式编译，指定参数：-DCMAKE_BUILD_TYPE=Release

如果机器上没有cmake工具，则需要先安装好。对于可连接外网，并有yum的机器，只需要执行：yum install cmake即可安装cmake。
