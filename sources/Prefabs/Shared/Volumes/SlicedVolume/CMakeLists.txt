set (SlicedVolume_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/SlicedVolume.cl
	${CMAKE_CURRENT_SOURCE_DIR}/SlicedVolume.h
	${CMAKE_CURRENT_SOURCE_DIR}/SlicedVolume.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/SlicedVolume.cpp
	PARENT_SCOPE)

set (SlicedVolume_KRNL
	#${CMAKE_CURRENT_SOURCE_DIR}/SlicedVolume.h
	${CMAKE_CURRENT_SOURCE_DIR}/SlicedVolume.cl
	PARENT_SCOPE)