cmake安装方法：
cmake -DCMAKE_INSTALL_PREFIX=<installation directory> .

对于依赖的第三方库，可以使用-D指定，
如：-DMYSQL_HOME=/home/mike/mysql，
默认在/usr/local等目录下寻找第三方库。

“-DCMAKE_INSTALL_PREFIX=”后跟安装目录，如/usr/local/mooon：
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/mooon .

如果以debug方式编译，指定参数：-DCMAKE_BUILD_TYPE=Debug
如果以Release方式编译，指定参数：-DCMAKE_BUILD_TYPE=Release

如果机器上没有cmake工具，则需要先安装好。对于可连接外网，并有yum的机器，只需要执行：yum install cmake即可安装cmake。

注意使用静态库时有顺序要求，假设静态库libx.a依赖于libz.a，则指定顺序须为-lx -lz（或libx.a libz.a），不能反过来为-lz -lx（或libz.a libx.a）。

