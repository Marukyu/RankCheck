add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Primitives")

set(SOURCES ${SOURCES} 
	"${CMAKE_CURRENT_SOURCE_DIR}/Style.cpp"	
	PARENT_SCOPE)
