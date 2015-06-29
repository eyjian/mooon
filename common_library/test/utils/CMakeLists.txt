include_directories(../../include)
link_directories(../../src)
link_libraries(libmooon.a)

add_executable(ut_string_utils ut_string_utils.cpp)
add_executable(ut_tokener ut_tokener.cpp)
