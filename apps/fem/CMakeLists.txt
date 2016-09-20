cmake_minimum_required(VERSION 2.8)
project(fem)

# Simit include files and library
if (NOT DEFINED ENV{SIMIT_INCLUDE_DIR} OR NOT DEFINED ENV{SIMIT_LIBRARY_DIR})
  message(FATAL_ERROR "Set the environment variables SIMIT_INCLUDE_DIR and SIMIT_LIBRARY_DIR")
endif ()
include_directories($ENV{SIMIT_INCLUDE_DIR})
find_library(simit simit $ENV{SIMIT_LIBRARY_DIR})

# compile the static scalar 2D equation solver
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(GLOB FEM2_SOURCE ${PROJECT_SOURCE_DIR}/static_scalar_2D.cpp)
add_executable(static_scalar_2D ${FEM2_SOURCE})
target_link_libraries(static_scalar_2D LINK_PUBLIC ${simit})

# compile the static scalar 3D equation solver
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(GLOB FEM3_SOURCE ${PROJECT_SOURCE_DIR}/static_scalar_3D.cpp)
add_executable(static_scalar_3D ${FEM3_SOURCE})
target_link_libraries(static_scalar_3D LINK_PUBLIC ${simit})
