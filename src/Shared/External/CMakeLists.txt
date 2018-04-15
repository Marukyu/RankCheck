find_package(RapidJSON QUIET)
add_library(rapidjson INTERFACE IMPORTED GLOBAL)
if(RapidJSON_FOUND)
	set_target_properties(rapidjson PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${RAPIDJSON_INCLUDE_DIRS})
else()
	set_target_properties(rapidjson PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include")
endif()

# We need a hack for CMAKE versions less than 3.11
# See https://stackoverflow.com/a/48390363/5086505 for details
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.11")
	find_package(pugixml QUIET)
	if(PUGIXML_FOUND)
		set_target_properties(pugixml PROPERTIES IMPORTED_GLOBAL TRUE)
	endif()
else()
	function(add_library)
		set(_args ${ARGN})
		if ("${_args}" MATCHES ";IMPORTED")
			list(APPEND _args GLOBAL)
		endif()
		_add_library(${_args})
	endfunction()
	
	find_package(pugixml QUIET)
endif()

# Build the external project and use the imported target pugixml to link against it
if(NOT pugixml_FOUND)
	include(ExternalProject)
	ExternalProject_Add(project_pugixml
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pugixml"
		CMAKE_ARGS
		"-G${CMAKE_GENERATOR}"
		"-DCMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES=${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}"
		"-DCMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES=${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}"
		"-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
		"-DCMAKE_CROSSCOMPILING_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
		"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
		"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
		"-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS}"
		INSTALL_COMMAND "")
	ExternalProject_Get_Property(project_pugixml BINARY_DIR)
	
	add_library(pugixml STATIC IMPORTED GLOBAL)
	set_target_properties(pugixml PROPERTIES
		IMPORTED_LOCATION "${BINARY_DIR}/libpugixml.a"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src")
	add_dependencies(pugixml project_pugixml)
endif()

add_library(tinyfiledialogs "tinyfiledialogs/tinyfiledialogs.c")
