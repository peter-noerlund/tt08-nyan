find_package(icestorm CONFIG)
find_package(nextpnr CONFIG)
find_package(yosys CONFIG)
find_package(PkgConfig)
find_package(fmt CONFIG)
find_package(lyra CONFIG)
find_package(asio CONFIG)

if(yosys_FOUND AND icestorm_FOUND AND nextpnr_FOUND)
    set(FPGA_CONSTRAINTS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../pico-ice/pico-ice.pcf)
    set(FPGA_TARGET_FREQ 25)

    add_custom_target(
        pico-ice-synth
        DEPENDS pico-ice.bin
    )

    add_custom_command(
        OUTPUT pico-ice.json
        COMMAND ${YOSYS_PATH} -q -p "synth_ice40 -device u -top top -spram -dsp -json pico-ice.json" -l pico-ice.log ${CMAKE_CURRENT_SOURCE_DIR}/../pico-ice/top.sv ${CMAKE_CURRENT_SOURCE_DIR}/../src/tt_um_nyan.sv ${CMAKE_CURRENT_SOURCE_DIR}/../src/graphics.sv ${CMAKE_CURRENT_SOURCE_DIR}/../src/music.sv pico-ice-pll.v
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../pico-ice/top.sv ${CMAKE_CURRENT_SOURCE_DIR}/../src/tt_um_nyan.sv ${CMAKE_CURRENT_SOURCE_DIR}/../src/graphics.sv ${CMAKE_CURRENT_SOURCE_DIR}/../src/music.sv pico-ice-pll.v
    )

    add_custom_command(
        OUTPUT pico-ice-pll.v
        COMMAND ${ICEPLL_PATH} -q -i 48 -o ${FPGA_TARGET_FREQ} -p -m -f pico-ice-pll.v
    )

    add_custom_command(
        OUTPUT pico-ice.asc
        COMMAND ${NEXTPNR_ICE40_PATH} -q --up5k --package sg48 --asc pico-ice.asc --json pico-ice.json --pcf ${FPGA_CONSTRAINTS_FILE} --top top
        DEPENDS ${FPGA_CONSTRAINTS_FILE} pico-ice.json
    )
    
    add_custom_command(
        OUTPUT pico-ice_timing.txt
        COMMAND ${ICETIME_PATH} -p ${FPGA_CONSTRAINTS_FILE} -r pico-ice_timing.txt -d up5k -c ${FPGA_TARGET_FREQ} pico-ice.asc
        DEPENDS pico-ice.asc
    )

    add_custom_command(
        OUTPUT pico-ice.bin
        COMMAND ${ICEPACK_PATH} pico-ice.asc pico-ice.bin
        DEPENDS pico-ice.asc pico-ice_timing.txt
    )
endif()

if(PkgConfig_FOUND)
    pkg_check_modules(libudev IMPORTED_TARGET "libudev>=221")
    pkg_check_modules(libmount IMPORTED_TARGET "mount")
endif()

if(PkgConfig_FOUND AND libudev_FOUND AND fmt_FOUND AND libmount_FOUND AND asio_FOUND)
    add_executable(pico-ice-install
        main.cpp
        pico_ice_installer.cpp)
    target_compile_features(pico-ice-install PRIVATE cxx_std_20)
    target_compile_options(pico-ice-install PRIVATE -Wall -W -Wextra -Wpedantic)
    target_link_libraries(pico-ice-install PRIVATE
        asio
        lyra
        fmt::fmt-header-only
        PkgConfig::libmount
        PkgConfig::libudev)
endif()