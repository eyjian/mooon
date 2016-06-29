# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

add_subdirectory(http_parser)
add_subdirectory(observer)
add_subdirectory(server)
add_subdirectory(dispatcher)
add_subdirectory(agent)

if (HAVE_THRIFT)
    add_subdirectory(db_proxy)
endif ()
