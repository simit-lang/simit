# Simit
set(SIMIT_SOURCE_DIRS . util)

foreach(dir ${SIMIT_SOURCE_DIRS})
  file(GLOB SIMIT_HEADERS ${SIMIT_HEADERS} ${dir}/*.h)
  file(GLOB SIMIT_SOURCES ${SIMIT_SOURCES} ${dir}/*.cpp)
endforeach()

set(SIMIT_HEADERS ${SIMIT_HEADERS})
set(SIMIT_SOURCES ${SIMIT_SOURCES})

add_subdirectory(backend)
add_subdirectory(frontend)
add_subdirectory(lower)
add_subdirectory(visualizer)

add_definitions(${SIMIT_DEFINITIONS})
include_directories(${SIMIT_INCLUDE_DIRS})
add_library(${PROJECT_NAME} ${SIMIT_LIBRARY_TYPE} ${SIMIT_HEADERS} ${SIMIT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${SIMIT_LIBRARIES})


# LLVM
if (DEFINED ENV{LLVM_CONFIG})
  set(LLVM_CONFIG $ENV{LLVM_CONFIG})
else()
  set(LLVM_CONFIG "llvm-config")
endif()

execute_process(COMMAND ${LLVM_CONFIG} --version RESULT_VARIABLE LLVM_CONFIG_RESULT OUTPUT_VARIABLE LLVM_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT ${LLVM_CONFIG_RESULT} STREQUAL "0")
  if (DEFINED ENV{LLVM_CONFIG})
    message(FATAL_ERROR "Could not execute llvm-config (LLVM_CONFIG=${LLVM_CONFIG}). Add llvm-config version 3.3 or higher to PATH or set the LLVM_CONFIG environment variable to point to it.")
  else()
    message(FATAL_ERROR "Could not execute llvm-config. Add llvm-config version 3.3 or higher to PATH or set the LLVM_CONFIG environment variable to point to it.")
  endif()
endif()

if (NOT ${LLVM_VERSION} MATCHES "^([3-9]|[0-9][0-9]+)\\.([3-9]|[0-9][0-9]+)(\\.[0-9]+)?(svn)?$")
  message(FATAL_ERROR "Need LLVM version 3.3 or higher. Found ${LLVM_CONFIG} version \"${LLVM_VERSION}\".")
elseif (${LLVM_VERSION} MATCHES "^([3-9]|[0-9][0-9]+)\\.([5-9]|[0-9][0-9]+)(\\.[0-9]+)?(svn)?$")
  set(LLVM_CONFIG_GT_45 1)
endif()

message("-- Found LLVM: ${LLVM_CONFIG} (version \"${LLVM_VERSION}\")")

string(SUBSTRING ${LLVM_VERSION} 0 1 LLVM_MAJOR_VERSION)
string(SUBSTRING ${LLVM_VERSION} 2 1 LLVM_MINOR_VERSION)
add_definitions("-DLLVM_MAJOR_VERSION=${LLVM_MAJOR_VERSION}")
add_definitions("-DLLVM_MINOR_VERSION=${LLVM_MINOR_VERSION}")

execute_process(COMMAND ${LLVM_CONFIG} --cppflags OUTPUT_VARIABLE LLVM_CPPFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_FLAGS ${LLVM_CPPFLAGS})

execute_process(COMMAND ${LLVM_CONFIG} --includedir OUTPUT_VARIABLE LLVM_INCLUDES OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories("${LLVM_INCLUDES}")

execute_process(COMMAND ${LLVM_CONFIG} --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
if (DEFINED LLVM_CONFIG_GT_45)
  execute_process(COMMAND ${LLVM_CONFIG} --system-libs OUTPUT_VARIABLE LLVM_SYSTEM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_LDFLAGS ${LLVM_LDFLAGS} ${LLVM_SYSTEM_LIBS})
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS ${LLVM_LDFLAGS})

set(LLVM_COMPONENTS core jit x86 bitwriter ipo)

execute_process(COMMAND ${LLVM_CONFIG} --libnames ${LLVM_COMPONENTS} OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(LLVM_LIBS)  # Turn the space-separate string into a ;-separated list
execute_process(COMMAND ${LLVM_CONFIG} --libdir OUTPUT_VARIABLE LLVM_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

foreach(lib ${LLVM_LIBS})
  find_library(libpath NAMES ${lib} NO_DEFAULT_PATH PATHS ${LLVM_LIBDIR} PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo)
  target_link_libraries(${PROJECT_NAME} ${libpath})
  unset(libpath CACHE)
endforeach()

separate_arguments(LLVM_LDFLAGS)
foreach(ldflag ${LLVM_LDFLAGS})
  string(STRIP ${ldflag} ldflag)
  if (${ldflag} MATCHES "^-l.*$")
	 string(LENGTH ${ldflag} len)
	 MATH(EXPR len ${len}-2)
	 string(SUBSTRING ${ldflag} 2 ${len} ldflag)
	 target_link_libraries(${PROJECT_NAME} ${ldflag})
  endif()
endforeach()


# EIGEN
if (DEFINED ENV{EIGEN3_INCLUDE_DIR})
  include_directories($ENV{EIGEN3_INCLUDE_DIR})
else ()
  find_package(Eigen3)
  if (EIGEN3_FOUND)
    add_definitions(-DEIGEN)
    include_directories( ${EIGEN3_INCLUDE_DIR} )
  endif()
endif()

