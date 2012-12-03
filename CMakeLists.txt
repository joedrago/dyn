cmake_minimum_required(VERSION 2.6)

project(dyn)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

if(UNIX)
    add_definitions(-g)
endif()

if(WIN32)
    add_definitions(/wd4996)
endif()

add_subdirectory(src)
add_subdirectory(tests)
