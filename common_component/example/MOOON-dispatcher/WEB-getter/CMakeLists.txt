include_directories(../../../include)
link_directories(../../../src/dispatcher)
link_directories(../../../src/http_parser)
link_libraries(libmooon_dispatcher.a)
link_libraries(libmooon_http_parser.a)
link_libraries(libmooon.a)

aux_source_directory(. SRCS)
add_executable(web_getter ${SRCS})
