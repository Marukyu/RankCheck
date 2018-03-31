add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Graphics")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/GUI3")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/RankCheck")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/System")

set(SOURCES ${SOURCES} 
	"${CMAKE_CURRENT_SOURCE_DIR}/MainClient.cpp"
	PARENT_SCOPE)
