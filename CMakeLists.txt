project(OpenRGB_cppSDK)

include_directories(include)
file(GLOB SOURCE_FILES "src/*.h" "src/*.cpp")

add_library(orgbsdk STATIC ${SOURCE_FILES})

install(TARGETS orgbsdk DESTINATION lib)
install(DIRECTORY include DESTINATION include)
