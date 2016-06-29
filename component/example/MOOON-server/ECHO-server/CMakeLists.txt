include_directories(../../../include)
link_directories(../../../src/server)
link_libraries(libmooon_server.a)
link_libraries(libmooon.a)

aux_source_directory(. SRCS)
add_executable(echod ${SRCS})
