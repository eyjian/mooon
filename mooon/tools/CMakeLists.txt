# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(.)
include_directories(../include)
include_directories(../include/mooon)
link_directories(../src/net)
link_directories(../src/sys)
link_directories(../src/utils)

# 计算md5工具
add_executable(md5 md5.cpp)
target_link_libraries(md5 libmooon_utils.a)

# 硬盘性能测试工具
add_executable(disk_benchmark disk_benchmark.cpp)
target_link_libraries(disk_benchmark libmooon_sys.a libmooon_utils.a)

if (MOOON_HAVE_LIBSSH2)
	# 远程命令工具
	add_executable(mooon_ssh mooon_ssh.cpp)
	target_link_libraries(mooon_ssh libmooon_net.a libmooon_sys.a libmooon_utils.a libssh2.a libcrypto.a)
	
	# 批量上传工具
	add_executable(mooon_upload mooon_upload.cpp)
	target_link_libraries(mooon_upload libmooon_net.a libmooon_sys.a libmooon_utils.a libssh2.a libcrypto.a)

    # 下载工具
    add_executable(mooon_download mooon_download.cpp)
    target_link_libraries(mooon_download libmooon_net.a libmooon_sys.a libmooon_utils.a libssh2.a libcrypto.a)
    
	# CMAKE_INSTALL_PREFIX
	install(
	        TARGETS mooon_ssh mooon_upload mooon_download
	        DESTINATION bin
	       )
endif ()

# r3c_stress
if (MOOON_HAVE_R3C)
    add_executable(r3c_stress r3c_stress.cpp)
    target_link_libraries(r3c_stress libmooon_sys.a libmooon_utils.a libr3c.a libhiredis.a)
    
    add_executable(redis_queue_mover redis_queue_mover.cpp)
    target_link_libraries(redis_queue_mover libmooon_sys.a libmooon_utils.a libr3c.a libhiredis.a)
    
    # CMAKE_INSTALL_PREFIX
    install(
            TARGETS r3c_stress redis_queue_mover
            DESTINATION bin
           )    
endif ()

# hbase_stress
if (MOOON_HAVE_THRIFT)
    exec_program(${THRIFT_HOME}/bin/thrift ARGS --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/hbase.thrift)
    exec_program(rm ARGS ${CMAKE_CURRENT_SOURCE_DIR}/THBaseService_server.skeleton.cpp)

    add_executable(hbase_stress hbase_stress.cpp THBaseService.cpp hbase_constants.cpp hbase_types.cpp)
    target_link_libraries(hbase_stress libmooon_sys.a libmooon_utils.a libthrift.a)
    
    add_executable(hbase_scan hbase_scan.cpp THBaseService.cpp hbase_constants.cpp hbase_types.cpp)
    target_link_libraries(hbase_scan libmooon_sys.a libmooon_utils.a libthrift.a)
    
    # CMAKE_INSTALL_PREFIX
    install(
            TARGETS hbase_stress hbase_scan
            DESTINATION bin
           )      
endif ()

# mysql_escape_test
if (MOOON_HAVE_MYSQL)    
    add_executable(mysql_escape_test mysql_escape_test.cpp)
    target_link_libraries(mysql_escape_test libmooon_sys.a libmooon_utils.a libmysqlclient.a)
    
    # CMAKE_INSTALL_PREFIX
    install(
            TARGETS mysql_escape_test
            DESTINATION bin
           ) 
endif ()

# CMAKE_INSTALL_PREFIX
install(
        TARGETS md5 disk_benchmark
        DESTINATION bin
       )
