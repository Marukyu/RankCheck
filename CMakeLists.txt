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

# Set executable output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set executable name
set(EXECUTABLE_NAME "RankCheck")

find_package(SFML 2.3 REQUIRED graphics window network system)
include_directories(${SFML_INCLUDE_DIR})

find_package(Poco 1.9 REQUIRED Foundation)
include_directories(${Poco_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

find_package(ZLIB REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("src")
add_subdirectory("res")

# Copy resource files
file(COPY ${RESOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
