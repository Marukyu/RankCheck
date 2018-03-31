set(SOURCES ${SOURCES}
	"${CMAKE_CURRENT_SOURCE_DIR}/PugiXML/pugixml.cpp"
	PARENT_SCOPE)

find_package(RapidJSON QUIET)
add_library(rapidjson INTERFACE IMPORTED GLOBAL)
if(RAPIDJSON_FOUND)
	set_target_properties(rapidjson PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${RAPIDJSON_INCLUDE_DIRS})
else()
	set_target_properties(rapidjson PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include")
endif()

add_library(tinyfiledialogs "tinyfiledialogs/tinyfiledialogs.c")
