add_executable(boncekeeb)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(boncekeeb ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(boncekeeb PRIVATE boncekeeb.c)

target_link_libraries(boncekeeb PRIVATE pico_stdlib hardware_pio)
pico_add_extra_outputs(boncekeeb)

# add url via pico_set_program_url
example_auto_set_url(boncekeeb)

# Additionally generate python and hex pioasm outputs for inclusion in the RP2040 datasheet
add_custom_target(boncekeeb_datasheet DEPENDS ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py)
add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
        COMMAND Pioasm -o python ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        )
add_dependencies(boncekeeb boncekeeb_datasheet)