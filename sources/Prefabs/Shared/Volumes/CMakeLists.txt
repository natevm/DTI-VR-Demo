add_subdirectory(SlicedVolume)
#add_subdirectory(RaycastVolume)

set (Volumes_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/Volume.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Volume.cpp
	${SlicedVolume_SRC}
#	${RaycastVolume_SRC}
	PARENT_SCOPE)

#set (Volumes_KRNL
#	${CMAKE_CURRENT_SOURCE_DIR}/Volume.h
#	${CMAKE_CURRENT_SOURCE_DIR}/Volume.cl
#	${SlicedVolume_KRNL}
#	${RaycastVolume_KRNL}
#	PARENT_SCOPE) 