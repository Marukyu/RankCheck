add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Config")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Content")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/External")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Utils")

set(SOURCES ${SOURCES} PARENT_SCOPE)
