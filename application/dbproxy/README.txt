cmake安装方法：
cmake -DCMAKE_INSTALL_PREFIX=<installation directory> .

“-DCMAKE_INSTALL_PREFIX=”后跟安装目录，如/usr/local/mooon：
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/mooon .

如果以debug方式编译，指定参数：-DCMAKE_BUILD_TYPE=Debug
如果以Release方式编译，指定参数：-DCMAKE_BUILD_TYPE=Release
如果机器上没有cmake工具，则需要先安装好。对于可连接外网，并有yum的机器，只需要执行：yum install cmake即可安装cmake。

db_proxy 访问DB的代理，通过配置SQL语句，提供访问DB的能力，为一个可执行应用。
db_proxy依赖thrift、mooon和mooon-observer
