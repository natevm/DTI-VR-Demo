# ┌──────────────────────────────────────────────────────────────────┐
# │  Aggregate source files                                          │
# └──────────────────────────────────────────────────────────────────┘

add_subdirectory(Options)
add_subdirectory(Prefabs)

set(DTI_DEMO_DESKTOP_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/DesktopMain.cpp
	${Options_SRC}
	${Prefabs_SRC}
	PARENT_SCOPE)

set(DTI_DEMO_VR_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/VRMain.cpp
	${Options_SRC}
	${Prefabs_SRC}
	PARENT_SCOPE)
