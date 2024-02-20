cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_lcd_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(hello_lcd main.cc)

add_subdirectory(../.. HD44780)

pico_add_extra_outputs(hello_lcd)

target_link_libraries(hello_lcd pico_stdlib HD44780)
