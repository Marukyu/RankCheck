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
	# Grab the rc file and attach it to the sources
	target_sources(${EXECUTABLE_NAME} PRIVATE "WinRes.rc")

	set(CMAKE_RC_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")
endif()

target_link_libraries(${EXECUTABLE_NAME} system config content pugixml)

# MinGW-specific workarounds
if(MINGW)
	# Link MinGW-specific dependencies.
	target_link_libraries(${EXECUTABLE_NAME} iphlpapi)
endif()

# Link POSIX Thread library if necessary
if(CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(${EXECUTABLE_NAME} pthread)
endif()
