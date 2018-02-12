cmake_minimum_required(VERSION 3.5)

# Set default build type to Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)")
endif()

project(RankCheck)

# Detect and enable C++11 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "RankCheck requires C++11, but ${CMAKE_CXX_COMPILER} does not appear to support C++11. Please use a different compiler or update ${CMAKE_CXX_COMPILER}.")
endif()

include_directories(src)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE RESOURCES "res/*")

# Set executable output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set executable name
set(EXECUTABLE_NAME "RankCheck")

# Add executable icon (Windows-specific)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	# Grab any RC files and attach them to sources list
	file(GLOB_RECURSE RC_SOURCES "src/*.rc")
	set(SOURCES ${SOURCES} ${RC_SOURCES})

	set(CMAKE_RC_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")

endif()

# Define executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Add CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Detect and add SFML
find_package(SFML 2.3 REQUIRED graphics window network system)
if(SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIR})
	target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

# Detect and add Poco
find_package(Poco REQUIRED)
if(Poco_FOUND)
	include_directories(${Poco_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${Poco_LIBRARIES})
endif()

# MinGW-specific workarounds
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

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
if(OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
endif()

# Detect and add zlib
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${EXECUTABLE_NAME} ${ZLIB_LIBRARIES})
endif()

# Link POSIX Thread library if necessary
if(CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(${EXECUTABLE_NAME} pthread)
endif()

# Copy resource files
file(COPY ${RESOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})