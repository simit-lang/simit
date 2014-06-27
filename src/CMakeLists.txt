set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

add_definitions("-D__STDC_LIMIT_MACROS")
add_definitions("-D__STDC_CONSTANT_MACROS")

if (SIMIT_SHARED_LIBRARY)
  add_definitions("-DHalide_SHARED")
  set(SIMIT_LIBRARY_TYPE SHARED)
else()
  set(SIMIT_LIBRARY_TYPE STATIC)
endif()

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(Simit ${SIMIT_LIBRARY_TYPE}
  ${SOURCES}
  ${HEADERS})

target_link_libraries(Simit ${LIBS})

