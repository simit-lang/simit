set(TESTS simit-test)

file(GLOB GTEST_SOURCE gtest/gtest-all.cc)
file(GLOB GTEST_HEADER gtest/gtest.h)
include_directories(${SIMIT_TEST_DIR})
add_library(gtest ${GTEST_HEADER} ${GTEST_SOURCE})

file(GLOB SOURCES *.cpp)
add_executable(${TESTS} ${SOURCES})
target_link_libraries(${TESTS} gtest)
target_link_libraries(${TESTS} pthread)

if ($ENV{SIMIT_TEST_VIZ})
  find_package(OpenGL)
  find_package(GLUT) 
  if (OPENGL_FOUND AND GLUT_FOUND)
    target_link_libraries(${TESTS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
    target_link_libraries(${TESTS} ${PROJECT_NAME}viz)
  endif ()
endif ()

target_link_libraries(${TESTS} ${PROJECT_NAME})

set(SIMIT_TEST_INPUT_DIR ${SIMIT_TEST_DIR}/input)
add_definitions(-DTEST_INPUT_DIR="${SIMIT_TEST_INPUT_DIR}")
add_definitions(-DEXAMPLES_DIR="${SIMIT_EXAMPLES_DIR}")
