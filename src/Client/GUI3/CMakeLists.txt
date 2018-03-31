add_library(gui3
	"Application.cpp"
	"Container.cpp"
	"Interface.cpp"
	"Piece.cpp"
	"Renderer.cpp"
	"ResourceManager.cpp"
	"Widget.cpp")

target_link_libraries(gui3 utils)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Events")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Models")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Pieces")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Rendering")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Utils")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Widgets")
