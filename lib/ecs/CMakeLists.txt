# ┌──────────────────────────────────────────────────────────────────┐
# │ Developer: n8vm                                                  |
# │  ECS - Entity Component System library                           |
# └──────────────────────────────────────────────────────────────────┘

include(${PROJECT_SOURCE_DIR}/CMakeGenerateHierarchy.txt)
add_subdirectory(Components)
add_subdirectory(Entities)
add_subdirectory(System)

set(ECS_SRC 
	${Components_SRC}
	${Entities_SRC}
	${System_SRC})

add_library(ECS STATIC ${ECS_SRC})

target_include_directories(ECS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPENVR_INCLUDE_DIR} ${TEEM_INCLUDE_DIR})
target_link_libraries(ECS PUBLIC GLDK ${OPENVR_LIBRARY} ${TEEM_LIBRARY})

# IDE Folder Hierarchy Generation
generate_folder_hierarchy("${ECS_SRC}")