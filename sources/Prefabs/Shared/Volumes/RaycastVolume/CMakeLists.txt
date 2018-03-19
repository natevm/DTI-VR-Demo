set (RaycastVolume_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/RaycastVolume.cl
	${CMAKE_CURRENT_SOURCE_DIR}/RaycastVolume.h
	${CMAKE_CURRENT_SOURCE_DIR}/RaycastVolume.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/RaycastVolume.cpp
	PARENT_SCOPE)

set (RaycastVolume_KRNL
	#${CMAKE_CURRENT_SOURCE_DIR}/RaycastVolume.h
	${CMAKE_CURRENT_SOURCE_DIR}/RaycastVolume.cl
	PARENT_SCOPE)