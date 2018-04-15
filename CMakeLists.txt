cmake_minimum_required(VERSION 3.5)

# Set default build type to Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)")
endif()

project(RankCheck)

set(BUILD_SHARED_LIBS FALSE)

# Add CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Detect and enable C++11 support
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
	# Use W4 and not Wall because apparently the standard library emits a large amount of warnings
	# cf. https://stackoverflow.com/questions/4001736/whats-up-with-the-thousands-of-warnings-in-standard-headers-in-msvc-wall#4001759
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wd4100")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# -Weverything is unused for similar reasons as above
	# Many of the warnings are stylistic, or superfluous (e.g. complaining about c++98 compat when we have --std=gnu++11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wno-unused-parameter")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter")
else()
	message(WARNING "Could not set the warning level (unknown compiler)")
endif()

# Set executable output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set executable name
set(EXECUTABLE_NAME "RankCheck")

find_package(SFML 2.3 REQUIRED graphics window network system)
include_directories(${SFML_INCLUDE_DIR})

find_package(Poco 1.9 REQUIRED Foundation)
include_directories(${Poco_INCLUDE_DIRS})
if(MINGW)
	# When using static Poco libraries, an additional preprocessor macro must be defined to prevent linking issues on MinGW.
	if(NOT DEFINED USE_STATIC_POCO_LIBS)
		set(USE_STATIC_POCO_LIBS false CACHE BOOL "Use static Poco libraries")
	endif()

	if(USE_STATIC_POCO_LIBS)
		foreach(library IN ${POCO_libraries})
			target_compile_definitions(${library} INTERFACE POCO_STATIC)
		endforeach()
	endif()
endif()

find_package(OpenGL REQUIRED)

find_package(ZLIB REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("src")
add_subdirectory("res")

# Copy resource files
file(COPY ${RESOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
