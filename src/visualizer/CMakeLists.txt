set(VIZ_PROJECT_NAME ${PROJECT_NAME}viz)

set(SIMIT_VISUALIZER_SOURCE_DIR ${SIMIT_SOURCE_DIR}/visualizer)
file(GLOB SOURCES *.cpp ${SIMIT_VISUALIZER_SOURCE_DIR}/*.cpp)
file(GLOB HEADERS *.h ${SIMIT_VISUALIZER_SOURCE_DIR}/*.h)
add_definitions(-DGL_GLEXT_PROTOTYPES)

add_library(${VIZ_PROJECT_NAME} ${SIMIT_LIBRARY_TYPE} ${HEADERS} ${SOURCES})

target_link_libraries(${VIZ_PROJECT_NAME} ${PROJECT_NAME})

# GL
target_link_libraries(${VIZ_PROJECT_NAME} GL)
target_link_libraries(${VIZ_PROJECT_NAME} GLU)
target_link_libraries(${VIZ_PROJECT_NAME} glut)
