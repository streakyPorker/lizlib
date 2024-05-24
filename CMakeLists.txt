cmake_minimum_required(VERSION 3.11)
project(lizlib VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
include(CheckIncludeFile)

find_package(fmt REQUIRED)
set(ROOT_PATH ${PROJECT_SOURCE_DIR})

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()


include_directories(include)


file(GLOB_RECURSE SRC_FILES src/*.cc src/*.cpp include/*.h)
add_library(lizlib SHARED ${SRC_FILES})
add_definitions(-DFMT_HEADER_ONLY)
# uring 必须要public

target_link_libraries(lizlib PUBLIC fmt pthread uring stdc++fs)


if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND LIZLIB_BUILD_TESTS)
    option(LIZLIB_BUILD_TESTS "build tests" ON)
else ()
    option(LIZLIB_BUILD_TESTS "build tests" OFF)
endif ()
if (LIZLIB_BUILD_TESTS)
    include(cmakes/gtest.cmake)
    enable_testing()
    file(GLOB_RECURSE TEST_SRC_FILES test/*.cc test/*.cpp)
    add_executable(unittest ${TEST_SRC_FILES})
    target_include_directories(unittest PRIVATE test)
    target_link_libraries(unittest PRIVATE gtest_main lizlib)
endif ()


add_executable(lizlib-exe main.cpp)
TARGET_LINK_LIBRARIES(lizlib-exe PRIVATE lizlib)
