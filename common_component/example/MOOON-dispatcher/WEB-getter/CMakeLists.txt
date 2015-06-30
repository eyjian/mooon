include_directories(../../include)
link_directories(..)
link_libraries(libmooon_dispatcher.a)
link_libraries(libmooon.a)

aux_source_directory(. SRCS)
add_executable(web_getter ${SRCS})
