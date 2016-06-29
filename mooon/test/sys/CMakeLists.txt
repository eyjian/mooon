# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
link_directories(../../src/sys)
link_directories(../../src/utils)
link_libraries(libmooon_sys.a)
link_libraries(libmooon_utils.a)

add_executable(test_safe_logger test_safe_logger.cpp)
add_executable(ut_datetime_utils ut_datetime_utils.cpp)
add_executable(ut_event_queue ut_event_queue.cpp)
add_executable(ut_fs_utils ut_fs_utils.cpp)
