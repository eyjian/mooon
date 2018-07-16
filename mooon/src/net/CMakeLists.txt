# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(.)
include_directories(../../include)
include_directories(../../include/mooon)
aux_source_directory(. SRCS)
add_library(mooon_net STATIC ${SRCS})

# CMAKE_INSTALL_PREFIX
install(
        DIRECTORY ../../include/mooon/net        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
