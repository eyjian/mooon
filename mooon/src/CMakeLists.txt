# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

add_subdirectory(utils)
add_subdirectory(sys)
add_subdirectory(net)

include_directories(../include)
include_directories(../include/mooon)

# libmooon.a
add_library(
    mooon
    ${MOOON_UTILS_SRC}
    ${MOOON_SYS_SRC}
    ${MOOON_NET_SRC}
)

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon
        DESTINATION lib
       )
