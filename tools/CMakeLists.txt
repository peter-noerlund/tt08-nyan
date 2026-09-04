
add_executable(make_bitmaps make_bitmaps.cpp)
target_compile_features(make_bitmaps PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(make_bitmaps PRIVATE -Wall -W -Wextra -Wpedantic)
endif()

add_executable(make_music make_music.cpp)
target_compile_features(make_music PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(make_music PRIVATE -Wall -W -Wextra -Wpedantic)
endif()