include(~/pico/pico-sdk/lib/tinyusb/hw/bsp/rp2040/family.cmake)


############################# start original ws2812 example 
add_executable(boncekeeb)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(boncekeeb ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(boncekeeb PRIVATE
boncekeeb.c
usb_descriptors.c
bonce_ssd1306.c
ssd1306.c
)

target_link_libraries(boncekeeb PRIVATE pico_stdlib hardware_pio hardware_dma hardware_i2c)
family_configure_device_example(boncekeeb)
pico_add_extra_outputs(boncekeeb)

# add url via pico_set_program_url
example_auto_set_url(boncekeeb)


# Additionally generate python and hex pioasm outputs for inclusion in the RP2040 datasheet
add_custom_target(boncekeeb_datasheet DEPENDS Pioasm ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py)
add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
        COMMAND Pioasm -o python ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        )
add_dependencies(boncekeeb boncekeeb_datasheet)


############################# ends original ws2812 example 


# # gets PROJECT name for the example (e.g. <BOARD>-<DIR_NAME>)
# family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})
# project(${PROJECT})
# # Checks this example is valid for the family and initializes the project
# family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})


# pico_generate_pio_header(boncekeeb ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)


# # Example include
# target_include_directories(${PROJECT} PUBLIC
#         ${CMAKE_CURRENT_SOURCE_DIR}/
#         )


# Configure compilation flags and libraries for the example... see the corresponding function
# in hw/bsp/FAMILY/family.cmake for details.

