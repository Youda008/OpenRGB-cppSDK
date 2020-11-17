project(OpenRGB_cppSDK)

include_directories(
	include
	shared/CppUtils-Essential
	shared/CppUtils-Network
)

file(GLOB SOURCE_FILES
	"src/*.h" "src/*.cpp"
	"shared/CppUtils-Essential/*.h" "shared/CppUtils-Essential/*.cpp"
	"shared/CppUtils-Network/*.h" "shared/CppUtils-Network/*.cpp"
)

add_library(orgbsdk STATIC ${SOURCE_FILES})

install(TARGETS orgbsdk DESTINATION lib)
install(DIRECTORY include DESTINATION include)
