# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
link_directories(../../src/net)
link_directories(../../src/sys)
link_directories(../../src/utils)
link_libraries(libmooon_net.a)
link_libraries(libmooon_sys.a)
link_libraries(libmooon_utils.a)

add_executable(udp_client_test udp_client_test.cpp)
add_executable(udp_server_test udp_server_test.cpp)
add_executable(ut_epollable_queue ut_epollable_queue.cpp)

if (MOOON_HAVE_LIBSSH2)
    add_executable(ut_libssh2 ut_libssh2.cpp)
    target_link_libraries(ut_libssh2 libssh2.a libssl.a libcrypto.a)
endif ()
