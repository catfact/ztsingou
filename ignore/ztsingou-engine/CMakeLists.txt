cmake_minimum_required(VERSION 3.15)

project(ztsingou)

set(CMAKE_CXX_STANDARD 14)


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -O2")


add_executable(ztsingou main.cpp)

target_link_libraries(ztsingou lo)
target_link_libraries(ztsingou jack)