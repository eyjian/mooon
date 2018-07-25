# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

# include路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/mooon/report_self)

# 执行cmake时，就编译好thrift文件
exec_program(${THRIFT_HOME}/bin/thrift ARGS --gen cpp -r -out ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/report_self.thrift)

# RPC
set(RPC_SRC ReportSelfService.cpp report_self_types.cpp report_self_constants.cpp)

# THRIFT_LIB
set(THRIFT_LIB libthriftnb.a libthrift.a libevent.a libssl.a libcrypto.a)

# libmooon_report_self.a
add_library(mooon_report_self STATIC ${RPC_SRC} report_self.cpp)

# report_self_server
add_executable(report_self_server ${RPC_SRC} report_self_server.cpp)
target_link_libraries(report_self_server libmooon.a libmysqlclient.a ${THRIFT_LIB})

# report_self_example
add_executable(report_self_example report_self_example.cpp)
target_link_libraries(report_self_example ${CMAKE_CURRENT_SOURCE_DIR}/libmooon_report_self.a libmooon.a ${THRIFT_LIB})

# 设置依赖关系
# report_self_example依赖libreport_self.a
ADD_DEPENDENCIES(report_self_example mooon_report_self)

# 安装
install(
        TARGETS mooon_report_self
        DESTINATION lib
       )

install(
        TARGETS report_self_server report_self_example
        DESTINATION bin
       )
