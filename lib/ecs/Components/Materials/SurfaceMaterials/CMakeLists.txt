add_subdirectory(Normal)
add_subdirectory(TexCoord)
add_subdirectory(Texture)
add_subdirectory(UniformColor)
add_subdirectory(VertexColor)

set(SurfaceMaterials_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${Normal_SRC}
	${TexCoord_SRC}
	${Texture_SRC}
	${UniformColor_SRC}
	${VertexColor_SRC}
	PARENT_SCOPE)