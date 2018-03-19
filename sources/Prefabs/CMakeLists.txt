add_subdirectory(Desktop)
add_subdirectory(VR)
add_subdirectory(Shared)

set(Prefabs_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${DESKTOP_SRC}
	${VR_SRC}
	${SHARED_SRC}
	PARENT_SCOPE)