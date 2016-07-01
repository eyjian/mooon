# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)
aux_source_directory(. SRCS)
add_library(mooon_dispatcher STATIC ${SRCS})

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_dispatcher
        DESTINATION lib
       )
install(
        DIRECTORY ../include/mooon/dispatcher        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
