cmake_minimum_required(VERSION 3.16)

project(minesweeper-tt08 LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)
find_package(verilator REQUIRED)

file(GLOB VERILOG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/hdl/*.v)

qt5_generate_moc(src/window.h ${CMAKE_CURRENT_BINARY_DIR}/moc_window.cpp)
qt5_generate_moc(src/monitor.h ${CMAKE_CURRENT_BINARY_DIR}/moc_monitor.cpp)

add_executable(minesweeper-tt08
    src/bitmap.cpp
    src/main.cpp
    src/monitor.cpp
    src/simulator.cpp
    src/window.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/moc_window.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/moc_monitor.cpp
)
target_include_directories(minesweeper-tt08 PRIVATE src)
target_compile_features(minesweeper-tt08 PRIVATE cxx_std_20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(minesweeper-tt08 PRIVATE -Wall -W -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare $<$<CONFIG:Release>:-march=native -flto>)
endif()
target_link_libraries(minesweeper-tt08 PRIVATE $<$<CONFIG:Release>:-flto>)
target_link_libraries(minesweeper-tt08 PRIVATE Qt5::Widgets Qt5::OpenGL)

verilate(minesweeper-tt08
    TOP_MODULE tt_um_minesweeper
    OPT_FAST "-O3 -march=native"
    SOURCES
    rtl/tt_um_minesweeper.v
)