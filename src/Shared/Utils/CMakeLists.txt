add_library(utils
	"Clipboard.cpp"
	"DataStream.cpp"
	"DebugLog.cpp"
	"Error.cpp"
	"FileChooser.cpp"
	"FPS.cpp"
	"Hash.cpp"
	"StringStream.cpp"
	"SystemMessage.cpp"
	"Timer.cpp"
	"Utilities.cpp"
	"Zlib.cpp")

target_link_libraries(utils tinyfiledialogs)
target_link_libraries(utils ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${Poco_LIBRARIES} ZLIB::ZLIB)

add_subdirectory("Debug")
add_subdirectory("Filesystem")
