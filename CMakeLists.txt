cmake_minimum_required(VERSION 3.0)

project(OpenRGB_cppSDK)

include_directories(
	include
	shared/CppUtils-Essential
	shared/CppUtils-Network
)

file(GLOB SOURCE_FILES
	"src/*.hpp" "src/*.cpp"
	"shared/CppUtils-Essential/*.hpp" "shared/CppUtils-Essential/*.cpp"
	"shared/CppUtils-Network/*.hpp" "shared/CppUtils-Network/*.cpp"
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_compile_definitions(DEBUG SAFETY_CHECKS)
else()
	#add_compile_definitions(CRITICALS_CATCHABLE)
endif()

add_library(orgbsdk STATIC ${SOURCE_FILES})

add_subdirectory(tools/orgbcli EXCLUDE_FROM_ALL)

find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_SOURCE_DIR}/doc/Doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile)
	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
		COMMENT "Generating API documentation with Doxygen"
	)
endif()

install(TARGETS orgbsdk DESTINATION lib)
install(TARGETS orgbcli DESTINATION bin)
install(DIRECTORY include DESTINATION include)
