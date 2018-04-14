add_library(gui3
	"Application.cpp"
	"Container.cpp"
	"Interface.cpp"
	"Piece.cpp"
	"Renderer.cpp"
	"ResourceManager.cpp"
	"Widget.cpp")

target_link_libraries(gui3 utils)
target_link_libraries(gui3 ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

add_subdirectory("Events")
add_subdirectory("Models")
add_subdirectory("Pieces")
add_subdirectory("Rendering")
add_subdirectory("Utils")
add_subdirectory("Widgets")
