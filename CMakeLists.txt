cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-g)

add_subdirectory(dynamic)
add_subdirectory(tests)
