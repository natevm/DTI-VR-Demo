add_subdirectory(HTCVive)

set (Cameras_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/CameraBufferObject.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/SpinTableCamera.hpp
	${HTCVive_SRC}
	PARENT_SCOPE)