cmake_minimum_required(VERSION 2.8)
project(lulesh)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wall")
file(GLOB COMMON_CODE ${PROJECT_SOURCE_DIR}/lulesh-util.cc ${PROJECT_SOURCE_DIR}/lulesh-init.cc)
file(GLOB SERIAL_CODE ${COMMON_CODE} ${PROJECT_SOURCE_DIR}/lulesh-serial.cc)
add_executable(${PROJECT_NAME}-serial ${SERIAL_CODE})
file(GLOB SIMIT_CODE ${COMMON_CODE} ${PROJECT_SOURCE_DIR}/lulesh-simit.cc)
add_executable(${PROJECT_NAME}-simit ${SIMIT_CODE})

# Simit include files and library
if (NOT DEFINED ENV{SIMIT_INCLUDE_DIR} OR NOT DEFINED ENV{SIMIT_LIB_DIR})
  message(FATAL_ERROR "Set the environment variables SIMIT_INCLUDE_DIR and SIMIT_LIB_DIR")
endif ()
include_directories($ENV{SIMIT_INCLUDE_DIR})
find_library(simit simit $ENV{SIMIT_LIB_DIR})
target_link_libraries(${PROJECT_NAME}-simit LINK_PUBLIC ${simit})
