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

add_subdirectory("Debug")
add_subdirectory("Filesystem")
