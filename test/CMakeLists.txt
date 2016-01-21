
set(gtest_force_shared_crt TRUE)
add_subdirectory(gtest-1.7.0)

set_target_properties(gtest_main PROPERTIES FOLDER "Test/gtest")
set_target_properties(gtest PROPERTIES FOLDER "Test/gtest")

set(TEST_SOURCES
	src/util.h src/util.cpp
)

if (IMGLOADER_WITH_LIBDDSIMG)
	set(TEST_SOURCES ${TEST_SOURCES} src/ddsimg.cpp)
endif()
if (IMGLOADER_WITH_PNG)
	set(TEST_SOURCES ${TEST_SOURCES} src/png.cpp)
endif()
if (IMGLOADER_WITH_STB_IMAGE)
	set(TEST_SOURCES ${TEST_SOURCES} src/stb_image.cpp)
endif()

# Link runTests with what we want to test and the GTest and pthread library
add_executable(imgload_test ${TEST_SOURCES})

target_compile_features(imgload_test PRIVATE cxx_nullptr)
set_target_properties(imgload_test PROPERTIES FOLDER "Test")

if (MSVC)
	# silence stupid warnings
	target_compile_definitions(imgload_test PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(imgload_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0/include")
target_link_libraries(imgload_test PRIVATE imageloader gtest_main)

target_compile_definitions(imgload_test PRIVATE "TEST_DATA_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

