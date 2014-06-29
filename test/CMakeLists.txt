include_directories ("${SIMIT_INCLUDE_DIR}")

if(DEFINED ENV{GTEST_ROOT})
	set(TESTS tests)
	set(GTEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/gtest)
	add_subdirectory($ENV{GTEST_ROOT} ${GTEST_BUILD_DIR})
	include_directories($ENV{GTEST_ROOT}/include)

	file(GLOB SOURCES *.cpp)
	add_executable(${TESTS} ${SOURCES})
	target_link_libraries(${TESTS} ${PROJECT_NAME})
	target_link_libraries(${TESTS} gtest)
	target_link_libraries(${TESTS} gtest_main)
else()
	message("Warning: The environment variable GTEST_ROOT is not defined so the Simit tests will not be built")
endif()

