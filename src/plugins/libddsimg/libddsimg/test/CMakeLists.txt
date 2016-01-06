
set(gtest_force_shared_crt TRUE)
add_subdirectory(gtest)

set(TEST_SOURCES
		src/ContextFixture.h
		src/ContextFixture.cpp

		src/context_tests.cpp
		src/image_tests.cpp src/test_data.cpp src/test_data.h)

# Link runTests with what we want to test and the GTest and pthread library
add_executable(dds_tests ${TEST_SOURCES})

target_include_directories(dds_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/gtest/include")
target_link_libraries(dds_tests PRIVATE ddsimg gtest_main)

target_compile_definitions(dds_tests PRIVATE "TEST_DATA_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

add_test(AllTestsInFoo dds_tests)

add_custom_target(run_tests COMMAND dds_tests)
