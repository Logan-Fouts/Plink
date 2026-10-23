cmake_minimum_required(VERSION 3.20)
project(firmware_tests C)

set(CMAKE_C_STANDARD 11)
enable_testing()

# Build Unity as a small library
add_library(unity STATIC ../lib/Unity/src/unity.c)
target_include_directories(unity PUBLIC ../lib/Unity/src)

add_executable(test_mux
    input/test_mux.c
    ../src/input/mux.c
    fakes/mux_hw_fake.c
    fakes/utils_hw_fake.c)
target_link_libraries(test_mux PRIVATE unity)
target_include_directories(test_mux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME test_mux COMMAND test_mux)


add_executable(test_button
    input/test_button.c
    ../src/input/button.c)
target_link_libraries(test_button PRIVATE unity)
target_include_directories(test_button PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME test_button COMMAND test_button)
