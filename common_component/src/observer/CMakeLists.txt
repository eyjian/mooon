# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)
aux_source_directory(. SRCS)
add_library(mooon_observer STATIC ${SRCS})

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_observer
        DESTINATION lib
       )
install(
        DIRECTORY ../../include/mooon/observer        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
