cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)
project(simit)

SET(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")

if (SIMIT_SHARED_LIBRARY)
  set(SIMIT_LIBRARY_TYPE SHARED)
else()
  set(SIMIT_LIBRARY_TYPE STATIC)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DOSX)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(OPTIMIZE "-O3" CACHE STRING "Optimization level")
set(C_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wmissing-declarations -Woverloaded-virtual -pedantic-errors -Wno-deprecated")
set(CMAKE_C_FLAGS "${C_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${C_CXX_FLAGS} -std=c++11 -Wno-deprecated-register")

set(SIMIT_SOURCE_DIR    ${CMAKE_CURRENT_LIST_DIR}/src)
set(SIMIT_TEST_DIR      ${CMAKE_CURRENT_LIST_DIR}/test)
set(SIMIT_TOOLS_DIR     ${CMAKE_CURRENT_LIST_DIR}/tools)
set(SIMIT_EXAMPLES_DIR  ${CMAKE_CURRENT_LIST_DIR}/examples)

set(SIMIT_INCLUDE_DIR ${SIMIT_SOURCE_DIR})
include_directories ("${SIMIT_INCLUDE_DIR}")
set(SIMIT_GENERATED_SOURCE_DIR ${SIMIT_SOURCE_DIR})
include_directories ("${SIMIT_GENERATED_SOURCE_DIR}")

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)
