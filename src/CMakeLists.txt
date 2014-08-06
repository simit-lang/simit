# Frontend
SET(PARSER ${SIMIT_GENERATED_SOURCE_DIR}/parser.cpp)
SET(TOKENS ${SIMIT_GENERATED_SOURCE_DIR}/tokens.h)
SET(SCANNER_HEADER ${SIMIT_GENERATED_SOURCE_DIR}/scanner.h)
SET(SCANNER ${SIMIT_GENERATED_SOURCE_DIR}/scanner.cpp)

IF($ENV{SIMIT_BUILD_PARSER})
	FIND_PACKAGE(BISON REQUIRED)
	SET(PARSER_INPUT ${SIMIT_SOURCE_DIR}/parser.ypp)
	IF(BISON_FOUND)
		ADD_CUSTOM_COMMAND(
			OUTPUT ${PARSER}
			COMMAND ${BISON_EXECUTABLE}
			        --no-lines
			        --defines=${TOKENS}
			        --output=${PARSER}
					  ${PARSER_INPUT}
			MAIN_DEPENDENCY ${PARSER_INPUT}
		COMMENT "Generating parser.cpp"
		)
	ENDIF()

	FIND_PACKAGE(FLEX REQUIRED)
	SET(SCANNER_INPUT ${SIMIT_SOURCE_DIR}/scanner.flex)
	IF(FLEX_FOUND)
		ADD_CUSTOM_COMMAND(
			OUTPUT ${SCANNER}
			COMMAND ${FLEX_EXECUTABLE}
			        --noline
			        --header-file=${SCANNER_HEADER}
			        --outfile=${SCANNER}
					  ${SCANNER_INPUT}
			MAIN_DEPENDENCY ${SCANNER_INPUT}
			COMMENT "Generating scanner.cpp"
		)
	ENDIF()
ENDIF()


# Simit
file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(${PROJECT_NAME} ${SIMIT_LIBRARY_TYPE} ${HEADERS} ${SOURCES} ${SCANNER} ${PARSER} ${SCANNER_INPUT} ${PARSER_INPUT})


# LLVM
if (DEFINED ENV{LLVM_CONFIG})
  set(LLVM_CONFIG $ENV{LLVM_CONFIG})
else()
  set(LLVM_CONFIG "llvm-config")
endif()
#message("- llvm-config: ${LLVM_CONFIG}")

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
endif()

message("-- Found LLVM: ${LLVM_CONFIG} (found version \"${LLVM_VERSION}\")")

execute_process(COMMAND ${LLVM_CONFIG} --cppflags OUTPUT_VARIABLE LLVM_CPPFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_FLAGS ${LLVM_CPPFLAGS})
#message("- cpp flags: ${LLVM_CPPFLAGS}")

execute_process(COMMAND ${LLVM_CONFIG} --includedir OUTPUT_VARIABLE LLVM_INCLUDES OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories("${LLVM_INCLUDES}")
#message("- include dir: ${LLVM_INCLUDES}")

execute_process(COMMAND ${LLVM_CONFIG} --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS ${LLVM_LDFLAGS})
#message("- ldflags: ${LLVM_LDFLAGS}")

set(LLVM_COMPONENTS core jit)
execute_process(COMMAND ${LLVM_CONFIG} --libnames ${LLVM_COMPONENTS} OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(LLVM_LIBS)  # Turn the space-separate string into a ;-separated list
execute_process(COMMAND ${LLVM_CONFIG} --libdir OUTPUT_VARIABLE LLVM_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

foreach(lib ${LLVM_LIBS})
  find_library(libpath NAMES ${lib} NO_DEFAULT_PATH PATHS ${LLVM_LIBDIR} PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo)
  #message(${libpath})
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
	 #message("- library: ${ldflag}")
	 target_link_libraries(${PROJECT_NAME} ${ldflag})
  endif()
endforeach()
