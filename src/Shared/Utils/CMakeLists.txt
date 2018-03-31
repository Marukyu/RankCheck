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

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Debug")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Filesystem")
