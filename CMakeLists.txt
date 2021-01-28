cmake_minimum_required(VERSION 3.12)

include(pico-sdk/pico_sdk_init.cmake)

project(rp2040_ov2640)

pico_sdk_init()

add_executable(firmware
	ov2640.c
	main.c
)

pico_generate_pio_header(firmware ${CMAKE_CURRENT_LIST_DIR}/image.pio)

target_link_libraries(firmware
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pio
	hardware_pwm
)

pico_add_extra_outputs(firmware)
