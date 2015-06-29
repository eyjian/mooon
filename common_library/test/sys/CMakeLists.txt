include_directories(../../include)
link_directories(../../src)
link_libraries(libmooon.a)

add_executable(test_safe_logger test_safe_logger.cpp)
add_executable(ut_datetime_utils ut_datetime_utils.cpp)
add_executable(ut_event_queue ut_event_queue.cpp)
add_executable(ut_fs_utils ut_fs_utils.cpp)
