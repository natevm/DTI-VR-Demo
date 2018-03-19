add_subdirectory(Textures)
add_subdirectory(Materials)
add_subdirectory(Meshes)
add_subdirectory(Math)

set (Components_SRC
	${Math_SRC}
	${Materials_SRC}
	${Meshes_SRC}
	${Textures_SRC}
	PARENT_SCOPE) 
