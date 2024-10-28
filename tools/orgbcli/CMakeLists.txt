include_directories(
	../../include
	../../external/CppUtils-Essential
)

file(GLOB SOURCE_FILES
	"src/*.hpp" "src/*.cpp"
	"../../external/CppUtils-Essential/*.hpp" "../../external/CppUtils-Essential/*.cpp"
)

add_executable(orgbcli ${SOURCE_FILES})
if (WIN32)
	target_link_libraries(orgbcli orgbsdk ws2_32)
else()
	target_link_libraries(orgbcli orgbsdk)
endif()
