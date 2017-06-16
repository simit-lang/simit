set(VIZ_PROJECT_NAME ${PROJECT_NAME}viz)

find_package(OpenGL)
find_package(GLUT)

if (NOT OPENGL_FOUND OR NOT GLUT_FOUND OR
    GLUT_Xi_LIBRARY STREQUAL GLUT_Xi_LIBRARY-NOTFOUND OR
    GLUT_Xmu_LIBRARY STREQUAL GLUT_Xmu_LIBRARY-NOTFOUND)
  message("Could not find OpenGL or GLUT so simviz will not be built")
else()
  message("-- Found OpenGL and GLUT")

  set(SIMIT_VISUALIZER_SOURCE_DIR ${SIMIT_SOURCE_DIR}/visualizer)
  file(GLOB SOURCES *.cpp ${SIMIT_VISUALIZER_SOURCE_DIR}/*.cpp)
  file(GLOB HEADERS *.h ${SIMIT_VISUALIZER_SOURCE_DIR}/*.h)
  add_definitions(-DGL_GLEXT_PROTOTYPES)

  add_library(${VIZ_PROJECT_NAME} ${SIMIT_LIBRARY_TYPE} ${HEADERS} ${SOURCES})

  target_link_libraries(${VIZ_PROJECT_NAME} ${PROJECT_NAME})

  include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
  target_link_libraries(${VIZ_PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

  # GL
  #include_directories(${OpenGL_INCLUDE_DIRS})
  #link_directories(${OpenGL_LIBRARY_DIRS})
  #add_definitions(${OpenGL_DEFINITIONS})

  #include_directories(${GLUT_INCLUDE_DIRS})
  #link_directories(${GLUT_LIBRARY_DIRS})
  #add_definitions(${GLUT_DEFINITIONS})

  #target_link_libraries(${VIZ_PROJECT_NAME} GL)
  #target_link_libraries(${VIZ_PROJECT_NAME} GLU)
  #target_link_libraries(${VIZ_PROJECT_NAME} glut)

  #include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})
endif()

# TACO
include_directories(${TACO_INCLUDE_DIR} ${TACO_SRC_DIR})
