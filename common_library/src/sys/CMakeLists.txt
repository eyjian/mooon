include_directories(../../include)
include_directories(../../include/mooon)
aux_source_directory(. SRCS)
add_library(mooon_sys STATIC ${SRCS})

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_sys
        DESTINATION lib
       )
install(
        DIRECTORY ../../include/mooon/sys        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
