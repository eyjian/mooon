# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

# 3.2
cmake_minimum_required(VERSION 2.8.11)
project(mooon)

include(CMake.common)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)

# CMAKE_INSTALL_PREFIX
install(
        FILES CMake.common        
        DESTINATION .
       )
