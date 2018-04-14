add_library(graphics
	"BitmapText.cpp"
	"FloatColor.cpp"
	"GradientRect.cpp"
	"TexturePacker.cpp"
	"UtilitiesSf.cpp")

target_link_libraries(graphics ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} OpenGL::GL)
