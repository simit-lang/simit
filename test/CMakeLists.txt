set(TESTS simit-test)
set(TESTS_F32 simit-test-f32)

file(GLOB GTEST_SOURCE gtest/gtest-all.cc)
file(GLOB GTEST_HEADER gtest/gtest.h)
include_directories(${SIMIT_TEST_DIR})
add_library(gtest ${GTEST_HEADER} ${GTEST_SOURCE})

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_executable(${TESTS} ${SOURCES} ${HEADERS})
target_link_libraries(${TESTS} gtest)
target_link_libraries(${TESTS} pthread)
target_link_libraries(${TESTS} ${PROJECT_NAME})

add_executable(${TESTS_F32} ${SOURCES})
target_link_libraries(${TESTS_F32} gtest)
target_link_libraries(${TESTS_F32} pthread)
set_target_properties(${TESTS_F32} PROPERTIES COMPILE_DEFINITIONS F32)
target_link_libraries(${TESTS_F32} ${PROJECT_NAME})

set(SIMIT_TEST_INPUT_DIR ${SIMIT_TEST_DIR}/input)
add_definitions(-DTEST_INPUT_DIR="${SIMIT_TEST_INPUT_DIR}")
add_definitions(-DEXAMPLES_DIR="${SIMIT_EXAMPLES_DIR}")

if (CUDA_FOUND)
  add_definitions(-DGPU)
endif ()

if ($ENV{SIMIT_TEST_VIZ})
  find_package(OpenGL)
  find_package(GLUT) 
  if (OPENGL_FOUND AND GLUT_FOUND)
    target_link_libraries(${TESTS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
    target_link_libraries(${TESTS} ${PROJECT_NAME}viz)
    target_link_libraries(${TESTS_F32} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
    target_link_libraries(${TESTS_F32} ${PROJECT_NAME}viz)
  endif ()
endif ()

