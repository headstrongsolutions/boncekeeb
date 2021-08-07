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

////////////////////////from pico examples - gpio - irq
// Rigging for keys
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

uint key_cols[5][2] = { {key_col1, 0}, {key_col2, 0}, {key_col3, 0}, {key_col4, 0}, {key_col5, 0} };
uint key_rows[4][2] = { {key_row1, 0}, {key_row2, 0}, {key_row3, 0}, {key_row4, 0} };

void setup_rows()
{
    // For each column, set gpio to read 
    for ( int i = 0; i <= key_col_count; ++i ){
        gpio_init(key_rows[i][0]);
        gpio_set_input_enabled (key_rows[i][0],1);
        gpio_set_dir (key_rows[i][0], GPIO_IN);
    }
}

void test_rows(uint gpio, uint32_t events) {
    // For each column in this row, test if high
    for (int i = 0; i  <= key_row_count; ++i){
        // test if this row is high
        int state = gpio_get(key_rows[i][0]); 

        if(state == 1)
        {
            key_rows[i][1] = 1;
            // set the corresponding led to white
        }
        else
        {
            key_rows[i][1] = 0;
            // set the corresponding led to off
        }
    }
    
}

void set_col_triggers() {
    int i;
    for ( i = 0; i < key_col_count; ++i ) {
        gpio_set_irq_enabled_with_callback( key_cols[i][0], GPIO_IRQ_EDGE_FALL, true, &test_rows);
    }
}


static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}
///////////////////////////// ends irq example


static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_single(uint len, uint t) {
    int pixel_array [20] = { 5, 10, 15, 20, 19,
                            14,  9,  4,  3,  8,
                            13, 18, 17, 12,  7,
                             2,  1,  6, 11, 16 };
    for (int i = 0; i < len; ++i) {
        if (pixel_array[i] >= 0 && pixel_array[i] <=5) {
            put_pixel(urgb_u32(0xff, 0xff, 0xff));
        }
        else if (pixel_array[i] > 5 && pixel_array[i] <=10) {
            put_pixel(urgb_u32(0xff, 0, 0));
        }
        else if (pixel_array[i] > 10 && pixel_array[i] <=15) {
            put_pixel(urgb_u32(0, 0xff, 0));
        }
        else if (pixel_array[i] > 15 && pixel_array[i] <=20) {
            put_pixel(urgb_u32(0, 0, 0xff));
        }
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