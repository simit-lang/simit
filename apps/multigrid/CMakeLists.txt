cmake_minimum_required(VERSION 2.8)
project(thermal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
file(GLOB SOURCE_CODE ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_CODE})

# Simit include files and library
if (NOT DEFINED ENV{SIMIT_INCLUDE_DIR} OR NOT DEFINED ENV{SIMIT_LIBRARY_DIR})
  message(FATAL_ERROR "Set the environment variables SIMIT_INCLUDE_DIR and SIMIT_LIBRARY_DIR")
endif ()
include_directories($ENV{SIMIT_INCLUDE_DIR})
find_library(simit simit $ENV{SIMIT_LIBRARY_DIR})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${simit})

# CGNS include files and library
include_directories($ENV{CGNS_INCLUDE_DIR})
if (NOT DEFINED ENV{CGNS_INCLUDE_DIR})
    find_library(cgns cgns "/usr/local/lib")
else ()
    find_library(cgns cgns $ENV{CGNS_LIB_DIR})
    include_directories($ENV{CGNS_LIB_DIR})
endif ()    
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${cgns})

# SILO include files and library
if (DEFINED ENV{VISIT})
    if (NOT DEFINED ENV{SILO_INCLUDE_DIR} OR NOT DEFINED ENV{SILO_LIB_DIR})
      message(FATAL_ERROR "Set the environment variables SILO_INCLUDE_DIR and SILO_LIB_DIR")
    endif ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVIZ_MESH")

    include_directories($ENV{SILO_INCLUDE_DIR})
    find_library(silo siloh5 $ENV{SILO_LIB_DIR})
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${silo})
endif ()
