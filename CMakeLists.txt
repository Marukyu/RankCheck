cmake_minimum_required(VERSION 3.5)

# Set default build type to Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)")
endif()

project(RankCheck)

# Detect and enable C++11 support
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)

# Set executable name
set(EXECUTABLE_NAME "RankCheck")

include_directories("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/res")

# Set executable output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set executable name
set(EXECUTABLE_NAME "RankCheck")

# Add executable icon (Windows-specific)
if(WIN32)
	# Grab any RC files and attach them to sources list
	file(GLOB_RECURSE RC_SOURCES "src/*.rc")
	set(SOURCES ${SOURCES} ${RC_SOURCES})

	set(CMAKE_RC_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")
endif()

# Define executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} system config content)

# Add CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Detect and add SFML
find_package(SFML 2.3 REQUIRED graphics window network system)
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

# Detect and add Poco
find_package(Poco REQUIRED)
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

# Copy resource files
file(COPY ${RESOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
