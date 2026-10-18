cmake_minimum_required(VERSION 3.13)

# set(PICO_BOARD pico2)   # uncomment for Pico 2 (must come before the include below)

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
)

target_include_directories(plink PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
)

target_link_libraries(plink
    pico_stdlib
    tinyusb_device
)

pico_add_extra_outputs(plink)