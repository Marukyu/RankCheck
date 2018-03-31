add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Controls")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Graphics")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Misc")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Panels")

target_sources(gui3 PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/TestWidget.cpp")
