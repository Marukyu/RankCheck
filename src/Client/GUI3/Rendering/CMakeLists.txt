add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Primitives")

target_sources(gui3 PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Style.cpp"	)
