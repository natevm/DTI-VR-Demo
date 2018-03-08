# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required (VERSION 3.10)
project(DTI_VR_Demo)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(${PROJECT_SOURCE_DIR}/CMakeGenerateHierarchy.txt)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘

# glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE STRING "" FORCE)
set(GLFW_INSTALL        OFF CACHE STRING "" FORCE)
add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)

# glew
add_definitions(-DGLEW_STATIC)
option(glew-cmake_BUILD_SHARED "" OFF)
add_subdirectory(lib/glew)
include_directories(lib/glew/include lib/glew/src)
find_package(GLEW REQUIRED)

# glm
find_package(GLM REQUIRED)

# gldk 
add_subdirectory(lib/gldk)

# openvr
find_package(OpenVR REQUIRED)

set(INCLUDES 
	lib/openvr/headers)

set(LIBRARIES
    ${OPENGL_LIBRARIES}
    ${OPENVR_LIBRARY}
    libglew_static
    glfw
    GLDK)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
add_executable (Desktop "./sources/main.cpp")
target_link_libraries (Desktop ${LIBRARIES})
target_include_directories(Desktop PUBLIC ${INCLUDES})

generate_folder_hierarchy(".")

