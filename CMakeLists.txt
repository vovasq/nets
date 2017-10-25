cmake_minimum_required(VERSION 3.8)
project(tcp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
#SET(GCC_COVERAGE_COMPILE_FLAGS "-fpermissive")
set(CMAKE_CXX_FLAGS "-fpermissive")
add_executable(tcp ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(tcp pthread)


