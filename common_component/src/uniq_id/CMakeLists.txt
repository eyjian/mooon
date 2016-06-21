# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# libmooon_uniq_id.a
add_library(mooon_uniq_id STATIC uniq_id.cpp)

# uniq_id_agent
add_executable(uniq_id_agent agent.cpp)
target_link_libraries(uniq_id_agent libmooon.a)

# uniq_cli
add_executable(uniq_cli uniq_cli.cpp)
target_link_libraries(uniq_cli libmooon_uniq_id.a libmooon.a)

# 设置依赖关系
ADD_DEPENDENCIES(uniq_cli mooon_uniq_id)

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_uniq_id
        DESTINATION lib
       )
install(
        DIRECTORY ../../include/mooon/uniq_id        
        DESTINATION include/mooon
        PATTERN "uniq_id.h"
       )
