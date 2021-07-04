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

add_library(orgbsdk STATIC ${SOURCE_FILES})

add_subdirectory(tools/orgbcli EXCLUDE_FROM_ALL)

install(TARGETS orgbsdk DESTINATION lib)
install(TARGETS orgbcli DESTINATION bin)
install(DIRECTORY include DESTINATION include)
