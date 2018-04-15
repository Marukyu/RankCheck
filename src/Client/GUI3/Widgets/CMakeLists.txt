add_subdirectory("Controls")
add_subdirectory("Graphics")
add_subdirectory("Misc")
add_subdirectory("Panels")

target_sources(gui3 PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/TestWidget.cpp")
