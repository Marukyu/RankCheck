add_library(system 
	"WOSApplication.cpp"
	"WOSInterface.cpp"
	"WOSMenu.cpp"
	"WOSRenderer.cpp"
	"WOSResourceManager.cpp")

target_link_libraries(system graphics gui3)
