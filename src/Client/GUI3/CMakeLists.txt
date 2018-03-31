add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Events")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Models")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Pieces")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Rendering")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Utils")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Widgets")

set(SOURCES ${SOURCES} 
	"${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Container.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Piece.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Widget.cpp"
	PARENT_SCOPE)
