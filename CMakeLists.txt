project(Simit)
cmake_minimum_required(VERSION 2.8.3)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(OPTIMIZE "-O3"
  CACHE STRING "Optimization level")

add_subdirectory(src)
add_subdirectory(test)

