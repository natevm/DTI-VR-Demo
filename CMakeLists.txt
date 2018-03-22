# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required (VERSION 3.10)
project(DTI_VR_Demo)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(${PROJECT_SOURCE_DIR}/CMakeGenerateHierarchy.txt)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

# Use c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Definitions                                                     │
# └──────────────────────────────────────────────────────────────────┘

add_definitions(-DResourcePath="./resources/")

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
include_directories(SYSTEM lib/glm)

# gli
find_package(GLI REQUIRED)
include_directories(SYSTEM lib/gli)

# teem 
find_package(Teem REQUIRED)
include_directories(SYSTEM ${TEEM_INCLUDE_DIR})

# gldk 
add_subdirectory(lib/gldk)

# openvr
find_package(OpenVR REQUIRED)
include_directories(SYSTEM ${OPENVR_INCLUDE_DIR})

#ecs
add_subdirectory(lib/ecs)

message(WARN ${TEEM_LIBRARY})

set(LIBRARIES
	${TEEM_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${OPENVR_LIBRARY}
    libglew_static
    glfw
    GLDK
    ECS)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Add source files                                                │
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(sources)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
add_executable (DTI_Demo_Desktop "${DTI_DEMO_DESKTOP_SRC}")
target_link_libraries (DTI_Demo_Desktop ${LIBRARIES})
generate_folder_hierarchy("${DTI_DEMO_DESKTOP_SRC}")

add_executable (DTI_Demo_VR "${DTI_DEMO_VR_SRC}")
target_link_libraries (DTI_Demo_VR ${LIBRARIES})
generate_folder_hierarchy("${DTI_DEMO_VR_SRC}")