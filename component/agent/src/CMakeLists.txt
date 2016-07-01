# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)
aux_source_directory(. SRCS)
add_library(mooon_agent STATIC ${SRCS})

add_subdirectory(center)
add_subdirectory(tester)

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_agent
        DESTINATION lib
       )
install(
        DIRECTORY ../include/mooon/agent        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
