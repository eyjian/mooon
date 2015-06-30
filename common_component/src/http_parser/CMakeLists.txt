include_directories(../../include)
include_directories(../../include/mooon)
aux_source_directory(. SRCS)
add_library(mooon_http_parser STATIC ${SRCS})

# CMAKE_INSTALL_PREFIX
install(
        TARGETS mooon_http_parser
        DESTINATION lib
       )
install(
        DIRECTORY ../../include/mooon/http_parser        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
