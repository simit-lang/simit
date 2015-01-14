# Parser and scanner
set(SIMIT_PARSER_SOURCE_DIR ${SIMIT_SOURCE_DIR}/parser)
include_directories ("${SIMIT_PARSER_SOURCE_DIR}")

#add_definitions(-DWITH_EXCEPTIONS)

SET(PARSER_INPUT   parser.ypp)
SET(PARSER_HEADER  parser.h)
SET(PARSER         parser.cpp)
SET(SCANNER_INPUT  scanner.flex)
SET(SCANNER_HEADER scanner.h)
SET(SCANNER        scanner.cpp)

FIND_PACKAGE(BISON 3.0.0)
if (BISON_FOUND)
  if ($ENV{SIMIT_PARSER_DEBUG})
    message("Warning: Parser files will have line directive absolute paths")
    ADD_CUSTOM_COMMAND(
      OUTPUT ${SIMIT_PARSER_SOURCE_DIR}/${PARSER}
      COMMAND ${BISON_EXECUTABLE}
              --defines=${SIMIT_PARSER_SOURCE_DIR}/${PARSER_HEADER}
              --output=${SIMIT_PARSER_SOURCE_DIR}/${PARSER}
              ${SIMIT_PARSER_SOURCE_DIR}/${PARSER_INPUT}
      MAIN_DEPENDENCY ${SIMIT_PARSER_SOURCE_DIR}/${PARSER_INPUT}
      COMMENT "Generating parser.cpp"
    )
  else()
    ADD_CUSTOM_COMMAND(
      OUTPUT ${SIMIT_PARSER_SOURCE_DIR}/${PARSER}
      COMMAND ${BISON_EXECUTABLE}
              --defines=${PARSER_HEADER}
              --output=${PARSER}
              --no-lines
              ${PARSER_INPUT}
      MAIN_DEPENDENCY ${SIMIT_PARSER_SOURCE_DIR}/${PARSER_INPUT}
      WORKING_DIRECTORY ${SIMIT_PARSER_SOURCE_DIR}
      COMMENT "Generating parser.cpp"
    )
  endif()
endif()

FIND_PACKAGE(FLEX 2.5.37 EXACT)
IF(FLEX_FOUND)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${SIMIT_PARSER_SOURCE_DIR}/${SCANNER}
    COMMAND ${FLEX_EXECUTABLE}
            --outfile=${SCANNER}
            ${SCANNER_INPUT}
    MAIN_DEPENDENCY ${SIMIT_PARSER_SOURCE_DIR}/${SCANNER_INPUT}
    WORKING_DIRECTORY ${SIMIT_PARSER_SOURCE_DIR}
    COMMENT "Generating scanner.cpp"
  )
ENDIF()

# Simit
file(GLOB SIMIT_HEADERS *.h ${SIMIT_PARSER_SOURCE_DIR}/${PARSER_HEADER} ${SIMIT_PARSER_SOURCE_DIR}/${SCANNER_HEADER} ${SIMIT_GRAPH_VIZ_SOURCE_DIR}/*.h)
file(GLOB SIMIT_SOURCES *.cpp ${SIMIT_PARSER_SOURCE_DIR}/*.cpp)

# GPU Backend
if (CUDA_FOUND)
  message("-- Building with gpu-backend")
  add_definitions(-DGPU)

  include_directories(${CUDA_INCLUDE_DIRS})
  include_directories("${CUDA_TOOLKIT_ROOT_DIR}/nvvm/include/")
  
  if (LINUX)
    get_filename_component(CUDA_LIB_DIR ${CUDA_LIBRARIES} DIRECTORY)
    add_definitions(-DLIBCUDADEVRT="${CUDA_LIB_DIR}/libcudadevrt.a")
  elseif (DARWIN)
    add_definitions(-DLIBCUDADEVRT="${CUDA_TOOLKIT_ROOT_DIR}/lib/libcudadevrt.a")
  endif ()

  set(SIMIT_GPU_SOURCE_DIR ${SIMIT_SOURCE_DIR}/gpu_backend)
  file(GLOB GPU_SOURCES ${SIMIT_GPU_SOURCE_DIR}/*.cpp)
  file(GLOB GPU_HEADERS ${SIMIT_GPU_SOURCE_DIR}/*.h)

  set(SIMIT_HEADERS ${SIMIT_HEADERS} ${GPU_HEADERS})
  set(SIMIT_SOURCES ${SIMIT_SOURCES} ${GPU_SOURCES})
else ()
  message("-- Building without gpu-backend")
endif ()

add_library(${PROJECT_NAME} ${SIMIT_LIBRARY_TYPE} ${SIMIT_HEADERS} ${SIMIT_SOURCES} ${SIMIT_PARSER_SOURCE_DIR}/${PARSER_INPUT} ${SIMIT_PARSER_SOURCE_DIR}/${SCANNER_INPUT})

# GPU Backend linking
if (CUDA_FOUND)
  if (LINUX)
    find_library(libcuda NAMES cuda PATHS "${CUDA_TOOLKIT_ROOT_DIR}/lib64")
    find_library(libnvvm NAMES nvvm PATHS "${CUDA_TOOLKIT_ROOT_DIR}/nvvm/lib64")
    target_link_libraries(${PROJECT_NAME} ${libcuda})
    target_link_libraries(${PROJECT_NAME} ${libnvvm})
  elseif (DARWIN)
    target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} "${CUDA_TOOLKIT_ROOT_DIR}/lib/libcuda.dylib")
    target_link_libraries(${PROJECT_NAME} "${CUDA_TOOLKIT_ROOT_DIR}/nvvm/lib/libnvvm.dylib")
  endif ()
endif ()

# Visualizer
add_subdirectory(visualizer)

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

message("-- Found LLVM: ${LLVM_CONFIG} (found version \"${LLVM_VERSION}\")")

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

set(LLVM_COMPONENTS core jit x86 bitwriter)
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

