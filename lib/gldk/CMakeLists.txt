# ┌──────────────────────────────────────────────────────────────────┐
# │ Developer: n8vm                                                  |
# │  GLDK is a library which takes care of glfw/glew stuff!          |
# └──────────────────────────────────────────────────────────────────┘

include(${PROJECT_SOURCE_DIR}/CMakeGenerateHierarchy.txt)

set(GLDK_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/gldk.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/gldk.cpp
	)

add_library(GLDK STATIC ${GLDK_SRC} )
target_include_directories(GLDK PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})