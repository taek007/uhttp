cmake_minimum_required(VERSION 3.1)
project(uhttp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS}")

set(SOURCE_FILES main.c)
add_executable(uhttp ${SOURCE_FILES} http.h http.c mem.h mem.c log.h server.h server.c fastcgi.h fastcgi.c)

target_link_libraries(uhttp
    pthread
)