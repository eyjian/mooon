# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

# 依赖thrift，必须存在
if (NOT HAVE_THRIFT)
    message("${Yellow}please set `-DTHIRFT_HOME=` value${ColourReset}")
    return ()
endif ()

include_directories(.)
include_directories(../..)
link_libraries(libmooon_observer.a libmooon.a)

# 执行cmake时，就编译好thrift文件
exec_program(${THRIFT_HOME}/bin/thrift ARGS --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/db_proxy.thrift)

#aux_source_directory(sql_writer SQL_WRITER_SRC)
set(RPC DbProxyService.cpp db_proxy_constants.cpp db_proxy_types.cpp)

# sqllog文件检测工具
add_executable(check_sqllog check_sqllog.cpp)
target_link_libraries(check_sqllog libmooon.a)

# dbproxy客户端工具
add_executable(db_proxy_client db_proxy_client.cpp ${RPC})
target_link_libraries(db_proxy_client libjsoncpp.a libthriftnb.a libthrift.a libevent.a)

# dbproxy压力测试工具
add_executable(db_proxy_stress db_proxy_stress.cpp ${RPC})
target_link_libraries(db_proxy_stress libjsoncpp.a libthriftnb.a libthrift.a libevent.a)

# db压力测试工具
add_executable(db_stress db_stress.cpp)

# db_proxy
add_executable(db_proxy ${RPC} main.cpp db_proxy_handler.cpp config_loader.cpp sql_logger.cpp db_process.cpp)

if (EXISTS ${MYSQL_HOME}/lib/libmysqlclient_r.a)
    target_link_libraries(db_proxy libjsoncpp.a libmysqlclient_r.a libthriftnb.a libthrift.a libevent.a)
    target_link_libraries(db_stress libmysqlclient_r.a)
elseif (EXISTS ${MYSQL_HOME}/lib/mysql/libmysqlclient_r.a)
    link_directories(${MYSQL_HOME}/lib/mysql)
    target_link_libraries(db_proxy libjsoncpp.a libmysqlclient_r.a libthriftnb.a libthrift.a libevent.a)
    target_link_libraries(db_stress libmysqlclient_r.a)
elseif (EXISTS ${MYSQL_HOME}/lib/mysql/libmysqlclient.a)
    link_directories(${MYSQL_HOME}/lib/mysql)
    target_link_libraries(db_proxy libjsoncpp.a libmysqlclient.a libthriftnb.a libthrift.a libevent.a)
    target_link_libraries(db_stress libmysqlclient.a)
else ()
    target_link_libraries(db_proxy libjsoncpp.a libmysqlclient.a libthriftnb.a libthrift.a libevent.a)
    target_link_libraries(db_stress libmysqlclient.a)
endif ()

# DBConnection依赖于SQLite3，虽然db_proxy并未使用SQLite3
if (HAVE_SQLITE3)
    target_link_libraries(db_proxy libsqlite3.a)
endif ()

# 供C++客户端使用的静态库
add_library(mooon_db_proxy STATIC ${RPC})

# 方便thrift文件更新时不需要从头执行cmake
add_custom_target(
                   db_proxy_rpc
                   DEPENDS db_proxy.thrift
                 )
add_custom_command(
                   TARGET db_proxy_rpc
                   PRE_BUILD                   
                   COMMAND thrift --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR} db_proxy.thrift
                  )

# CMAKE_INSTALL_PREFIX
install(
        TARGETS db_proxy db_proxy_client db_proxy_stress db_stress
        DESTINATION bin
       )

install(
        TARGETS mooon_db_proxy
        DESTINATION lib
       )

install(
        FILES db_proxy.thrift
        DESTINATION include/mooon/db_proxy
       )

install(
        FILES DbProxyService.h db_proxy_types.h db_proxy_constants.h
        DESTINATION include/mooon/db_proxy
       )
