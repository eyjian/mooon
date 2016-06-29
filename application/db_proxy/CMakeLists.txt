# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(.)
include_directories(../..)
link_libraries(libmooon_observer.a libmooon.a)

# 执行cmake时，就编译好thrift文件
exec_program(mkdir ARGS -p ${CMAKE_CURRENT_SOURCE_DIR}/rpc)
exec_program(${THRIFT_HOME}/bin/thrift ARGS --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR}/rpc ${CMAKE_CURRENT_SOURCE_DIR}/db_proxy.thrift)

#aux_source_directory(sql_writer SQL_WRITER_SRC)
set(RPC rpc/DbProxyService.cpp rpc/db_proxy_constants.cpp rpc/db_proxy_types.cpp)
add_executable(db_proxy main.cpp db_proxy_handler.cpp config_loader.cpp sql_logger.cpp ${RPC})
target_link_libraries(db_proxy libjsoncpp.a libmysqlclient_r.a libthriftnb.a libthrift.a libevent.a)

add_executable(db_proxy-tester tester.cpp ${RPC})
target_link_libraries(db_proxy-tester libjsoncpp.a libthriftnb.a libthrift.a libevent.a)

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
                   COMMAND mkdir -p ./rpc
                   COMMAND thrift --gen cpp -r -out ./rpc db_proxy.thrift
                  )

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_db_proxy
        DESTINATION lib
       )

install(
        FILES db_proxy.thrift
        DESTINATION include/mooon/db_proxy
       )
install(
        FILES rpc/DbProxyService.h rpc/db_proxy_types.h rpc/db_proxy_constants.h
        DESTINATION include/mooon/db_proxy/rpc
       )
