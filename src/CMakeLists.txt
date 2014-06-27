add_definitions("-D__STDC_LIMIT_MACROS")
add_definitions("-D__STDC_CONSTANT_MACROS")
add_definitions("-std=c++11")

if (SIMIT_SHARED_LIBRARY)
  add_definitions("-DHalide_SHARED")
  set(SIMIT_LIBRARY_TYPE SHARED)
else()
  set(SIMIT_LIBRARY_TYPE STATIC)
endif()

set(HEADER_FILES
  Simit.h)

add_library(Simit ${SIMIT_LIBRARY_TYPE}
  Simit.cpp
  ${HEADER_FILES})

target_link_libraries(Simit ${LIBS})

