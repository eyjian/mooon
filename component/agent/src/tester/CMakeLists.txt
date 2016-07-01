# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(../../include)
link_directories(..)
link_libraries(libmooon_agent.a)
link_libraries(libmooon.a)

add_executable(tester main.cpp)
