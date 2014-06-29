project(Simit)
cmake_minimum_required(VERSION 2.8.3)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(OPTIMIZE "-O3" CACHE STRING "Optimization level")

set(SIMIT_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(SIMIT_INCLUDE_DIR ${SIMIT_SRC_DIR})

add_subdirectory(src)
add_subdirectory(test)

