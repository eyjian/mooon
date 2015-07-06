# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

cmake_minimum_required(VERSION 3.2)
project(mooon)

include(CMake.common)

add_subdirectory(src)
add_subdirectory(test)

# CMAKE_INSTALL_PREFIX
install(
        FILES CMake.common        
        DESTINATION .
       )
