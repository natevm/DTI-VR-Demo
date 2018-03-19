add_subdirectory(SurfaceMaterials)
add_subdirectory(VolumeMaterials)

set (Materials_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Material.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Materials.hpp
	${SurfaceMaterials_SRC}
	${VolumeMaterials_SRC}
	PARENT_SCOPE) 