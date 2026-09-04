find_package(Qt5 COMPONENTS Widgets OpenGL)
find_package(verilator)

if(Qt5_FOUND AND verilator_FOUND)
    qt5_generate_moc(window.h ${CMAKE_CURRENT_BINARY_DIR}/moc_window.cpp)
    qt5_generate_moc(monitor.h ${CMAKE_CURRENT_BINARY_DIR}/moc_monitor.cpp)

    add_executable(tt08-nyan
        main.cpp
        monitor.cpp
        simulator.cpp
        window.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/moc_window.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/moc_monitor.cpp
    )
    target_include_directories(tt08-nyan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_features(tt08-nyan PRIVATE cxx_std_20)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(tt08-nyan PRIVATE -Wall -W -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare $<$<CONFIG:Release>:-march=native -flto>)
    endif()
    target_link_libraries(tt08-nyan PRIVATE $<$<CONFIG:Release>:-flto>)
    target_link_libraries(tt08-nyan PRIVATE Qt5::Widgets Qt5::OpenGL)

    verilate(tt08-nyan
        TOP_MODULE tt_um_nyan
        OPT_FAST "-O3 -march=native"
        VERILATOR_ARGS -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/../src
        SOURCES
        ../src/tt_um_nyan.sv
        ../src/graphics.sv
        ../src/music.sv
    )
endif()