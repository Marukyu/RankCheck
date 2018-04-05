add_executable(${EXECUTABLE_NAME} 
	"CountryLookup.cpp"
	"GameFolder.cpp"
	"GameLogReader.cpp"
	"LeagueReader.cpp"
	"LogReader.cpp"
	"MainClient.cpp"
	"NautsNames.cpp"
	"NetworkLogReader.cpp"
	"NetworkLogStartupReader.cpp"
	"PersistentLogReader.cpp"
	"PlayerCard.cpp"
	"PlayerData.cpp"
	"PlayerDB.cpp"
	"RankChecker.cpp"
	"RankCheckWidget.cpp"
	"RatingHistoryEntry.cpp"
	"ReplayParser.cpp"
	"ReplayWatcher.cpp"
	"UsernameLookup.cpp")

# Add executable icon (Windows-specific)
if(WIN32)
	# Grab any the rc file and attach it to the sources
	target_sources(${EXECUTABLE_NAME} PRIVATE "WinRes.rc")

	set(CMAKE_RC_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")
endif()

target_link_libraries(${EXECUTABLE_NAME} system config content rapidjson pugixml)

# Detect and add SFML
find_package(SFML 2.3 REQUIRED graphics window network system)
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

# Detect and add Poco
find_package(Poco 1.9 REQUIRED Foundation)
include_directories(${Poco_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME} ${Poco_LIBRARIES})

# MinGW-specific workarounds
if(MINGW)
	# When using static Poco libraries, an additional preprocessor macro must be defined to prevent linking issues on MinGW.
	if(NOT DEFINED USE_STATIC_POCO_LIBS)
		set(USE_STATIC_POCO_LIBS false CACHE BOOL "Use static Poco libraries")
	endif()

	if(USE_STATIC_POCO_LIBS)
		target_compile_definitions(${EXECUTABLE_NAME} PRIVATE POCO_STATIC)
	endif()

	# Link MinGW-specific dependencies.
	target_link_libraries(${EXECUTABLE_NAME} iphlpapi)
endif()

# Detect and add OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} OpenGL::GL)

# Detect and add zlib
find_package(ZLIB REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} ZLIB::ZLIB)

# Link POSIX Thread library if necessary
if(CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(${EXECUTABLE_NAME} pthread)
endif()
