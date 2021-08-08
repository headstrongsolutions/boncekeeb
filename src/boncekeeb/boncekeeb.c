/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"


#define total_key_count 20

#define key_col_count 5
#define key_row_count 4

#define key_col0 13
#define key_col1 12
#define key_col2 11
#define key_col3 10
#define key_col4 9

#define key_row0 22
#define key_row1 26
#define key_row2 27
#define key_row3 28


int key_cols[5] = { key_col0, key_col1, key_col2, key_col3, key_col4 };
int key_rows[4] = { key_row0, key_row1, key_row2, key_row3 };

int key_xy[key_row_count][key_col_count] = {
    { 0, 1, 2, 3, 4}, 
    { 5, 6, 7, 8, 9}, 
    {10,11,12,13,14}, 
    {15,16,17,18,19}
};

typedef struct{
    int key_index;
    int led_index;
    int rgb[3];
    int state;
    char name[15];
} Keeb_Key;

Keeb_Key keeb_keys[total_key_count] = {
    {0, 16, {0xff, 0x00, 0x00},0, "ESC"},
    {1, 15, {0xff, 0x00, 0x00},0, "0"},
    {2, 8, {0xff, 0x00, 0x00},0, "9"},
    {3, 7, {0xff, 0x00, 0x00},0, "5"},
    {4, 0, {0xff, 0x00, 0x00},0, "6"},

    {5, 17, {0x00, 0xff, 0x00},0, "["},
    {6, 14, {0x00, 0xff, 0x00},0, "]"},
    {7, 9, {0x00, 0xff, 0x00},0, "T"},
    {8, 6, {0x00, 0xff, 0x00},0, "O"},
    {9, 1, {0x00, 0xff, 0x00},0, "I"},

    {10, 18, {0x00, 0x00, 0xff},0, "L"},
    {11, 13, {0x00, 0x00, 0xff},0, ";"},
    {12, 10, {0x00, 0x00, 0xff},0, "H"},
    {13, 5, {0x00, 0x00, 0xff},0, "W"},
    {14, 2, {0x00, 0x00, 0xff},0, "E"},

    {15, 19, {0xff, 0xff, 0xff},0, "."},
    {16, 12, {0xff, 0xff, 0xff},0, "\'"},
    {17, 11, {0xff, 0xff, 0xff},0, "A"},
    {18, 4, {0xff,0xff, 0xff},0, "S"},
    {19, 3, {0xff, 0xff, 0xff},0, "D"}
};

void setup_rows()
{
    // For each row, set gpio to read
    for ( int i = 0; i < key_row_count; ++i ){
        gpio_init(key_rows[i]);
        gpio_set_input_enabled (key_rows[i],1);
        gpio_set_dir (key_rows[i], GPIO_IN);
    }
}

void setup_cols()
{
    // For each column, set gpio to write
    for ( int i = 0; i <= key_col_count; ++i ){
        gpio_init(key_cols[i]);
        gpio_set_dir(key_cols[i], true);
        gpio_put(key_cols[i], false);
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

Keeb_Key find_key_by_gpio(int col, int row_gpio){
    int row;
    int key_index = -1;

    switch (row_gpio){
        case key_row0:
            row = 0;
            break;
        case key_row1:
            row = 1;
            break;
        case key_row2:
            row = 2;
            break;
        case key_row3:
            row = 3;
            break;
        default:
            row = -1;
            break;
    }
    if (col > -1 && row > -1){
        key_index = key_xy[row][col];
    }
    return keeb_keys[key_index];
}

void test_rows(uint col) {
    // For each row in this row, test if high
    for (int i = 0; i  < key_row_count; ++i){
        // test if this row is high
        bool row_state = gpio_get(key_rows[i]);
        sleep_ms(1);
        Keeb_Key keeb_key = find_key_by_gpio(col, key_rows[i]);
        keeb_keys[keeb_key.key_index].state = row_state;
        if (keeb_keys[keeb_key.key_index].state == 1){
            keeb_keys[keeb_key.key_index].rgb[0] = 0;
            keeb_keys[keeb_key.key_index].rgb[1] = 0;
            keeb_keys[keeb_key.key_index].rgb[2] = 0;
            // TODO now needs a timeout for repeater
        }
    }
}

void scan_cols() {
    // set each col high, then test underlying rows
    for(int i = 0; i < key_col_count; ++i){
        // set col high
        gpio_put(key_cols[i], true);

        // test rows
        test_rows(i);

        // set col low
        gpio_put(key_cols[i], false);
        sleep_ms(1);
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
    //set_sys_clock_48();
    stdio_init_all();

    // bind irq events
    setup_cols();
    setup_rows();

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, PIN_TX, 800000, false);

    //int t = 0;
    while (1) {
        scan_cols();
        // int pat = rand() % count_of(pattern_table);
        // int dir = (rand() >> 30) & 1 ? 1 : -1;
        // puts(pattern_table[pat].name);
        // puts(dir == 1 ? "(forward)" : "(backward)");
        // for (int i = 0; i < 1000; ++i) {
        //     pattern_table[pat].pat(20, 255);
        //     sleep_ms(10);
        //     t += dir;
        // }
    }
}