add_library(config 
	"ConfigAggregator.cpp"
	"Config.cpp"
	"ConfigSource.cpp"
	"JSONConfig.cpp"
	"NullConfig.cpp"
	"Paths.cpp")

target_link_libraries(config rapidjson)
