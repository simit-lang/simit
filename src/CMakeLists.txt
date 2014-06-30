set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

add_definitions("-D__STDC_LIMIT_MACROS")
add_definitions("-D__STDC_CONSTANT_MACROS")

if (SIMIT_SHARED_LIBRARY)
  set(SIMIT_LIBRARY_TYPE SHARED)
else()
  set(SIMIT_LIBRARY_TYPE STATIC)
endif()

# Frontend
FIND_PACKAGE(BISON REQUIRED)
SET(PARSER Parser.cpp)
IF(BISON_FOUND)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${PARSER}
		COMMAND ${BISON_EXECUTABLE}
		        --defines=${SIMIT_GENERATED_SOURCE_DIR}/Tokens.h
		        --output=${PARSER}
				  ${SIMIT_SOURCE_DIR}/Parser.ypp
	COMMENT "Generating parser.cpp"
	)
ENDIF()

FIND_PACKAGE(FLEX REQUIRED)
SET(SCANNER Scanner.cpp)
IF(FLEX_FOUND)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${SCANNER}
		COMMAND ${FLEX_EXECUTABLE}
		        --outfile=${SCANNER}
				  ${SIMIT_SOURCE_DIR}/Scanner.flex
		COMMENT "Generating scanner.cpp"
	)
ENDIF()

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(Simit ${SIMIT_LIBRARY_TYPE} ${SOURCES} ${PARSER} ${SCANNER} ${HEADERS})

target_link_libraries(Simit ${LIBS})

