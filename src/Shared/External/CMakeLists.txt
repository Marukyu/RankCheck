set(SOURCES ${SOURCES}
	"${CMAKE_CURRENT_SOURCE_DIR}/PugiXML/pugixml.cpp"
	PARENT_SCOPE)

add_library(tinyfiledialogs "tinyfiledialogs/tinyfiledialogs.c")
