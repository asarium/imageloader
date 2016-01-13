
SET(CPP_SOURCES imgload/Context.cpp imgload/Context.hpp imgload/Exception.cpp imgload/Exception.hpp imgload/Image.cpp imgload/Image.hpp)

add_library(imageloaderpp STATIC ${CPP_SOURCES})

target_link_libraries(imageloaderpp PUBLIC imageloader)

target_include_directories(imageloaderpp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_features(imageloaderpp PUBLIC cxx_auto_type cxx_deleted_functions cxx_noexcept)
