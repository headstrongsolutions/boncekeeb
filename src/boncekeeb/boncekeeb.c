/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define total_key_count 20
#define key_col_count 5
#define key_col1 13
#define key_col2 12
#define key_col3 11
#define key_col4 10
#define key_col5 9

#define key_row_count 4
#define key_row1 22
#define key_row2 26
#define key_row3 27
#define key_row4 28

int key_cols[5] = { key_col1, key_col2, key_col3, key_col4, key_col5 };
int key_rows[4] = { key_row1, key_row2, key_row3, key_row4 };

int key_xy[key_row_count][key_col_count] = {
    {0,1,2,3,4}, {5,6,7,8,9} ,{10,11,12,13,14},{15,16,17,18,19}
};


typedef struct{
    int key_index;
    int led_index;
    int rgb[3];
    int state;
} Keeb_Key;

Keeb_Key keeb_keys[total_key_count] = {
    {0, 16, {0xff, 0x00, 0x00},0},
    {1, 15, {0xff, 0x00, 0x00},0},
    {2, 8, {0xff, 0x00, 0x00},0},
    {3, 7, {0xff, 0x00, 0x00},0},
    {4, 0, {0xff, 0x00, 0x00},0},

    {5, 17, {0x00, 0xff, 0x00},0},
    {6, 14, {0x00, 0xff, 0x00},0},
    {7, 9, {0x00, 0xff, 0x00},0},
    {8, 6, {0x00, 0xff, 0x00},0},
    {9, 1, {0x00, 0xff, 0x00},0},

    {10, 18, {0x00, 0x00, 0xff},0},
    {11, 13, {0x00, 0x00, 0xff},0},
    {12, 10, {0x00, 0x00, 0xff},0},
    {13, 5, {0x00, 0x00, 0xff},0},
    {14, 2, {0x00, 0x00, 0xff},0},

    {15, 19, {0xff, 0xff, 0xff},0},
    {16, 12, {0xff, 0xff, 0xff},0},
    {17, 11, {0xff, 0xff, 0xff},0},
    {18, 4, {0xff,0xff, 0xff},0},
    {19, 3, {0xff, 0xff, 0xff},0}
};

void setup_rows()
{
    // For each column, set gpio to read 
    for ( int i = 0; i <= key_col_count; ++i ){
        gpio_init(key_rows[i]);
        gpio_set_input_enabled (key_rows[i],1);
        gpio_set_dir (key_rows[i], GPIO_IN);
    }
}

int find_key_by_led_index(int led_index){
    
    for(int i = 0; i < sizeof(keeb_keys) / sizeof(Keeb_Key); ++i){
        if(keeb_keys[i].led_index == led_index){
            return keeb_keys[i].key_index;
        }
    }
    return -1;
}

Keeb_Key find_key_by_gpio(int col, uint row_gpio){
    int row;
    int key_index = -1;

    switch (row_gpio){
        case key_row1:
            row = 1;
            break;
        case key_row2:
            row = 2;
            break;
        case key_row3:
            row = 3;
            break;
        case key_row4:
            row = 4;
            break;
        default:
            row = -1;
            break;
    }
    if (col > -1 && row > -1){
        key_index = key_xy[col][row];
    }

    return keeb_keys[key_index];
}

void test_rows(uint row_gpio, uint32_t events) {
    // For each row in this row, test if high
    for (int i = 0; i  <= key_row_count; ++i){
        // test if this row is high
        int state = gpio_get(key_rows[i]); 
        
        Keeb_Key keeb_key = find_key_by_gpio(i, row_gpio);
        keeb_keys[keeb_key.key_index].state = state;
    }
}

void set_col_triggers() {
    int i;
    for ( i = 0; i < key_col_count; ++i ) {
        gpio_set_irq_enabled_with_callback( key_cols[i], GPIO_IRQ_EDGE_FALL, true, &test_rows);
    }
}

static inline void put_pixel(uint32_t pixel_grb) {
    // send the RGB value for a LED pixel
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    // create a uint32 from 3 hex values for a hex based RGB colour
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_single(uint len, uint t) {
    // Loops through all keeb_keys
    for (int i = 0; i < sizeof(keeb_keys) / sizeof(Keeb_Key); ++i) {
        // use the i incremented index as a point of reference to get the keeb_key
        int keeb_key_index = find_key_by_led_index(i);
        // then use that keys RGB values to set the LED
        put_pixel(
            urgb_u32(
                keeb_keys[keeb_key_index].rgb[0], 
                keeb_keys[keeb_key_index].rgb[1], 
                keeb_keys[keeb_key_index].rgb[2]
            )
        );
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_single, "Single"},
};

const int PIN_TX = 0;

int main() {
    // bind irq events
    set_col_triggers();

    //set_sys_clock_48();
    stdio_init_all();
    puts("WS2812 Smoke Test");

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, PIN_TX, 800000, false);

    int t = 0;
    while (1) {
        int pat = rand() % count_of(pattern_table);
        int dir = (rand() >> 30) & 1 ? 1 : -1;
        puts(pattern_table[pat].name);
        puts(dir == 1 ? "(forward)" : "(backward)");
        for (int i = 0; i < 1000; ++i) {
            pattern_table[pat].pat(20, 255);
            sleep_ms(10);
            t += dir;
        }
    }
}