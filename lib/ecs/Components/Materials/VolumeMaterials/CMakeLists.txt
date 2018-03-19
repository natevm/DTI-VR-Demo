add_subdirectory(ProxyGeoVolume)

set(VolumeMaterials_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${ProxyGeoVolume_SRC}
	PARENT_SCOPE)