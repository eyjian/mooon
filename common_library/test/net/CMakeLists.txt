include_directories(../../include)
link_directories(../../src)
link_libraries(libmooon.a)

add_executable(udp_client_test udp_client_test.cpp)
add_executable(udp_server_test udp_server_test.cpp)
add_executable(ut_epollable_queue ut_epollable_queue.cpp)
