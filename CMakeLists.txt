cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(DDCKV LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED)

include_directories(src)
include_directories("/usr/include")

link_directories("/usr/lib/x86_64-linux-gnu")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")



add_subdirectory(src)

add_subdirectory(ycsb-test)
add_subdirectory(micro-test)

enable_testing()
add_subdirectory(tests)