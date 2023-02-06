cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(spi_lmh6401_project C CXX ASM)

pico_sdk_init()

add_executable(spi_lmh6401 spi_lmh6401.c)

target_link_libraries(spi_lmh6401
	pico_stdlib
	hardware_spi
)

pico_enable_stdio_usb(spi_lmh6401 1)
pico_enable_stdio_uart(spi_lmh6401 0)

pico_add_extra_outputs(spi_lmh6401)
