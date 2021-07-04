include_directories(
	../../include
	../../shared/CppUtils-Essential
)

file(GLOB SOURCE_FILES
	"src/*.hpp" "src/*.cpp"
	"../../shared/CppUtils-Essential/*.hpp" "../../shared/CppUtils-Essential/*.cpp"
)

add_executable(orgbcli ${SOURCE_FILES})
target_link_libraries(orgbcli orgbsdk ws2_32)
