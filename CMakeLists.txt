project(OpenRGB_cppSDK)

include_directories(
	include
	external/CppUtils-Essential
	external/CppUtils-Network
)

file(GLOB SOURCE_FILES
	"src/*.h" "src/*.cpp"
	"external/CppUtils-Essential/*.h" "external/CppUtils-Essential/*.cpp"
	"external/CppUtils-Network/*.h" "external/CppUtils-Network/*.cpp"
)

add_library(orgbsdk STATIC ${SOURCE_FILES})

install(TARGETS orgbsdk DESTINATION lib)
install(DIRECTORY include DESTINATION include)
