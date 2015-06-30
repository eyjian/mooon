include_directories(../../include)
link_directories(../../server)
link_libraries(libmooon_server.a)
link_libraries(libmooon.a)

add_executable(center main.cpp)

