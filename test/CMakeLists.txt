
set(TESTS test)
file(GLOB SOURCES *.cpp)
add_executable(${TESTS} ${SOURCES})
target_link_libraries(${TESTS} ${PROJECT_NAME})

if(DEFINED ENV{GTEST_ROOT})
	set(GTEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/gtest)
	add_subdirectory($ENV{GTEST_ROOT} ${GTEST_BUILD_DIR})
	include_directories($ENV{GTEST_ROOT}/include)
	target_link_libraries(${TESTS} gtest)
	target_link_libraries(${TESTS} gtest_main)
else()
	message("GTEST_ROOT not set so will attempt to check out gtest 1.7.0 using SVN")
	include(ExternalProject)
	set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/ThirdParty)
	ExternalProject_Add(
		googletest
		SVN_REPOSITORY http://googletest.googlecode.com/svn/tags/release-1.7.0
		SVN_REVISION -r 664
		TIMEOUT 10
		# Force separate output paths for debug and release builds to allow easy
		# identification of correct lib in subsequent TARGET_LINK_LIBRARIES commands
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		           -Dgtest_force_shared_crt=ON
		# Disable install step
		INSTALL_COMMAND ""
		# Wrap download, configure and build steps in a script to log output
		LOG_DOWNLOAD ON
		LOG_CONFIGURE ON
		LOG_BUILD ON)
	ExternalProject_Get_Property(googletest source_dir)
	include_directories(${source_dir}/include)
	ExternalProject_Get_Property(googletest binary_dir)

	if(MSVC)
		set(SUFFIX ".lib")
	else()
		set(SUFFIX ".a")
	endif()

	if (CMAKE_GENERATOR STREQUAL "Xcode")
		target_link_libraries(${TESTS} ${binary_dir}/Debug/libgtest${SUFFIX})
		target_link_libraries(${TESTS} ${binary_dir}/Debug/libgtest_main${SUFFIX})
	else()
		target_link_libraries(${TESTS} ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${SUFFIX})
		target_link_libraries(${TESTS} ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${SUFFIX})
	endif()
endif()

