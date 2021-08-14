add_executable(boncekeeb_ws2812)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(boncekeeb_ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(boncekeeb_ws2812 PRIVATE boncekeeb_ws2812.c bonce_ssd1306.c ssd1306.c)

target_link_libraries(boncekeeb_ws2812 PRIVATE pico_stdlib hardware_pio hardware_dma hardware_i2c)
pico_add_extra_outputs(boncekeeb_ws2812)

# add url via pico_set_program_url
example_auto_set_url(boncekeeb_ws2812)


# Additionally generate python and hex pioasm outputs for inclusion in the RP2040 datasheet
add_custom_target(boncekeeb_ws2812_datasheet DEPENDS Pioasm ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py)
add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
        COMMAND Pioasm -o python ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        )
add_dependencies(boncekeeb_ws2812 boncekeeb_ws2812_datasheet)

