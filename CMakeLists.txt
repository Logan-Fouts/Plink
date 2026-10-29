# So far im not really a fan of the VScode pico extension, So I'm commenting this out for now
# # == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
# if(WIN32)
#     set(USERHOME $ENV{USERPROFILE})
# else()
#     set(USERHOME $ENV{HOME})
# endif()
# set(sdkVersion 2.3.1)
# set(toolchainVersion 15_2_Rel1)
# set(picotoolVersion 2.3.1)
# set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# if (EXISTS ${picoVscode})
#     include(${picoVscode})
# endif()
# # ====================================================================================

cmake_minimum_required(VERSION 3.13)

# Environment must point at this sdk path
include(pico_sdk_import.cmake)

project(plink C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(plink
    src/main.c
    src/usb/usb_descriptors.c
    src/midi/midi_out.c
    src/debug/debug.c
    src/input/button.c
    src/input/keys.c
    src/input/mux.c
    src/input/controls.c
    src/input/push_button.c
    src/leds/leds.c
    src/hardware/rp2040/mux_hw_rp2040.c
    src/hardware/rp2040/utils_hw_rp2040.c
    src/hardware/rp2040/leds_hw_rp2040.c
)

pico_generate_pio_header(plink ${CMAKE_CURRENT_LIST_DIR}/src/leds/ws2812.pio)

target_include_directories(plink PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
)

target_link_libraries(plink
    pico_stdlib
    tinyusb_device
    hardware_adc
    hardware_pio
    pico_multicore
)

pico_add_extra_outputs(plink)
