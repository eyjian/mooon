# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
include_directories(../../include/mooon)

# 源代码
set(
    MOOON_SYS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/curl_wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main_template.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mysql_db.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/safe_logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared_memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3_db.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mem_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pool_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/semaphore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signal_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall_exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dir_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fs_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/read_write_lock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared_library.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_db.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cpp
    CACHE INTERNAL
    MOOON_SYS_SRC
)

# libmooon_sys.a
add_library(
    mooon_sys
    STATIC
    ${MOOON_SYS_SRC}
)

# CMAKE_INSTALL_PREFIX
install(
        DIRECTORY ../../include/mooon/sys        
        DESTINATION include/mooon
        PATTERN "*.h"
       )
