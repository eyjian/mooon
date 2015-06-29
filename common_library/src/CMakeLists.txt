add_subdirectory(utils)
add_subdirectory(sys)
add_subdirectory(net)

include_directories(../include)
include_directories(../include/mooon)
aux_source_directory(utils UTILS_SRCS)
aux_source_directory(sys SYS_SRCS)
aux_source_directory(net NET_SRCS)
add_library(mooon STATIC ${UTILS_SRCS} ${SYS_SRCS} ${NET_SRCS})

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon
        DESTINATION lib
       )
