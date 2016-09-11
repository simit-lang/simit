cmake_minimum_required(VERSION 2.8)
project(fem)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(GLOB FEM2_SOURCE ${PROJECT_SOURCE_DIR}/fem2.cpp)
add_executable(fem2 ${FEM2_SOURCE})

# Simit include files and library
if (NOT DEFINED ENV{SIMIT_INCLUDE_DIR} OR NOT DEFINED ENV{SIMIT_LIBRARY_DIR})
  message(FATAL_ERROR "Set the environment variables SIMIT_INCLUDE_DIR and SIMIT_LIBRARY_DIR")
endif ()
include_directories($ENV{SIMIT_INCLUDE_DIR})
find_library(simit simit $ENV{SIMIT_LIBRARY_DIR})
target_link_libraries(fem2 LINK_PUBLIC ${simit})
