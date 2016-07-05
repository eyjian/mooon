# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# libmooon_uniq.a
add_library(mooon_uniq STATIC uniq_id.cpp)

# uniq_agent
add_executable(uniq_agent agent.cpp)
target_link_libraries(uniq_agent libmooon.a)

if (HAVE_MYSQL)
	# uniq_master
	add_executable(uniq_master master.cpp)
	target_link_libraries(uniq_master libmooon.a libmysqlclient.a)
endif ()

# uniq_cli
add_executable(uniq_cli uniq_cli.cpp)
target_link_libraries(uniq_cli libmooon_uniq.a libmooon.a)

# master_cli
add_executable(master_cli master_cli.cpp)
target_link_libraries(master_cli libmooon_uniq.a libmooon.a)

# 设置依赖关系
ADD_DEPENDENCIES(uniq_cli mooon_uniq)
ADD_DEPENDENCIES(master_cli mooon_uniq)

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_uniq
        DESTINATION lib
       )
install(
        DIRECTORY ../include/mooon/uniq_id       
        DESTINATION include/mooon
        PATTERN "uniq_id.h"
       )
install(
        TARGETS uniq_agent
        DESTINATION bin
       )
if (HAVE_MYSQL)
    install(
            TARGETS uniq_master
            DESTINATION bin
           )
endif ()
install(
        TARGETS uniq_cli
        DESTINATION bin
       )
install(
        TARGETS master_cli
        DESTINATION bin
       )
