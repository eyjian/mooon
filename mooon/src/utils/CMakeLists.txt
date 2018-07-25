# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)

# 源代码
set(
    MOOON_UTILS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/aes_helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/args_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bit_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/charset_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_format_exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/integer_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/md5.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/md5_helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string_utils.cpp
    CACHE INTERNAL
    MOOON_UTILS_SRC
)

# libmooon_utils.a
add_library(
    mooon_utils
    STATIC
    ${MOOON_UTILS_SRC}
)

# CMAKE_INSTALL_PREFIX
install(
        DIRECTORY ../../include/mooon/utils        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
