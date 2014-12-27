file(GLOB UTIL_SOURCES "${SIMIT_TOOLS_DIR}/*.cpp")

# GPU backend (TEMP)
find_package(CUDA)
if (CUDA_FOUND)
  add_definitions(-DGPU)
endif ()

foreach(UTIL_SOURCE ${UTIL_SOURCES})
	get_filename_component(UTIL ${UTIL_SOURCE} NAME_WE)
	add_executable(${UTIL} ${UTIL_SOURCE})
	target_link_libraries(${UTIL} ${PROJECT_NAME})
endforeach()
