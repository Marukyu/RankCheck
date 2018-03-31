cmake_minimum_required(VERSION 3.5)

# Set default build type to Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)")
endif()

project(RankCheck)

# Add CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Detect and enable C++11 support
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)

# Set executable output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set executable name
set(EXECUTABLE_NAME "RankCheck")

include_directories("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/res")

# Copy resource files
file(COPY ${RESOURCES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
